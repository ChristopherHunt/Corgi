#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include "policy/quorum/swing/swing_quorum.h"
#include "utility/utils/utils.h"

SwingQuorum::SwingQuorum(Node *node) {
   MPI_ASSERT(node != NULL);
   this->node = node;
   this->buf = node->buf;
   this->msg_info = &(node->msg_info);
   this->status = &(node->status);
}

SwingQuorum::~SwingQuorum() {
}

void SwingQuorum::handle_put() {
   fprintf(stderr, "SwingQuorum handle_put not implemented!\n");
   MPI_ASSERT(FAILURE);

}

void SwingQuorum::handle_put_ack() {
   fprintf(stderr, "SwingQuorum handle_put_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_put_local() {
#ifdef DEBUG
   fprintf(stderr, "SwingQuorum handle_put_local\n");
   print_msg_info(&(node->msg_info));
#endif
   int result;

   recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, PUT_LOCAL,
         msg_info->comm, status);

   PutTemp put;
   PutTemplate *format = (PutTemplate *)buf;
   put.unpack(format);

   JobNodeID id;
   id.job_num = put.job_num;
   id.job_node = put.job_node;
   id.cache_node = put.cache_node;

   // If the swing node did not have a mapping of the key to any nodes, add
   // the key mapping.
   if (node->key_to_nodes.count(put.key) == 0) {
#ifdef DEBUG
      fprintf(stderr, "----- Swing node %d did not have key %s in its key_to_nodes!\n",
            node->local_rank, put.key.c_str());
#endif
      std::vector<JobNodeID> v;
      v.push_back(id);
      node->key_to_nodes[put.key] = v;
   }
   // If the swing node had a mapping of the key to some nodes, ensure that the
   // new node that has the key is included in that mapping.
   else {
#ifdef DEBUG
      fprintf(stderr, "+++++ Swing node %d had key %s in its key_to_nodes!\n",
            node->local_rank, key.c_str());
#endif
      std::vector<JobNodeID> v = node->key_to_nodes[put.key];
      if (std::find(v.begin(), v.end(), id) == v.end()) {
         node->key_to_nodes[put.key].push_back(id);
      }

   }

#ifdef DEBUG
   std::vector<JobNodeID> v = node->key_to_nodes[put.key];
   for (std::vector<JobNodeID>::iterator it = v.begin();
         it != v.end(); ++it) {
      fprintf(stderr, "Swing node %d sees vector for key %s containing node %d of job %d with cache_node %d\n",
            node->local_rank, put.key.c_str(), it->job_node, it->job_num, it->cache_node);
   }
#endif

   MPI_Comm swing_comm = node->job_to_comms[id.job_num].swing;
   MPI_ASSERT(swing_comm != MPI_COMM_NULL);

   // If this message came from a cache node
   if (msg_info->comm != swing_comm) {
      // Forward this to all other swing nodes so everyone is current regarding
      // the key/value mapping.
      for (int i = 0; i < node->local_size; ++i) {
         // Don't want to send the put to this swing node!
         if (node->local_rank != i) {
#ifdef DEBUG
            fprintf(stderr, "SwingNode %d sending PUT for key %s to SwingNode %d!\n",
                  node->local_rank, put.key.c_str(), i);
#endif
            result = send_msg(buf, sizeof(PutTemplate), MPI_UINT8_T, i,
                  msg_info->tag, swing_comm, &request);
            MPI_ASSERT(result == MPI_SUCCESS);
         }
      }
   }

   // This is currently setup to immediately return without having the other
   // swing nodes propogate the updates to their child cache nodes.
   // Only reply if you are a swing node replying back to a cache node.
   if (msg_info->comm != swing_comm) {
      result = send_msg(buf, sizeof(PutAckTemplate), MPI_UINT8_T, msg_info->src,
            PUT_LOCAL_ACK, msg_info->comm, &request);
      MPI_ASSERT(result == MPI_SUCCESS);
   }
}

// TODO: Shouldn't need this, but include for completeness
void SwingQuorum::handle_put_local_ack() {
#ifdef DEBUG
   fprintf(stderr, "SwingQuorum handle_put_local_ack\n");
#endif
   int result;

   result = recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src,
                     PUT_LOCAL_ACK, msg_info->comm, status);
   MPI_ASSERT(result == MPI_SUCCESS);

   fprintf(stderr, "SwingQuorum handle_put_local_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_get() {
#ifdef DEBUG
   fprintf(stderr, "SwingQuorum handle_get\n");
   print_msg_info(&(node->msg_info));
#endif

   recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, GET,
         msg_info->comm, status);

   int result;
   uint32_t votes_req;

   GetTemp get;
   GetTemplate *format = (GetTemplate *)buf;
   get.unpack(format);
   //uint32_t job_num = format->job_num;
   //uint32_t job_node = format->job_node;
   //uint32_t key_size = format->key_size;
   //std::string key(format->key, format->key + key_size);

   // Grab the list of nodes which have a value associated with the target key.
   std::vector<JobNodeID> nodes = node->key_to_nodes[get.key];

   // If nobody has the key/value, then set votes to 0 so the caller knows this
   // key does not exist in the cache. 
   if (nodes.size() == 0) {
      votes_req = 0;
   }
   // Otherwise, set the votes required to the proper number of nodes.
   // TODO: Make this not just W = 1, R = N! Add flexibility later.
   else {
      votes_req = nodes.size();
   }

   // Build the new message to send to the calling cache node (a lot of this is
   // reassignment but its safer for now to do it this way, we can strip out
   // the reassihnmnet to the buffer later). The main point of this message is
   // to tell the calling cache node how many of its peers need to vote on this
   // GetReq before it can decide it has enough data to choose the correct
   // value based on timestamps.
   CensusTemp census;
   CensusTemplate *census_format = (CensusTemplate *)buf;
   census.pack(census_format, get.job_num, get.job_node, get.key, votes_req);
   //census_format->job_num = job_num;
   //census_format->job_node = job_node;
   //census_format->key_size = key_size;
   //memcpy(census_format->key, key.c_str(), key_size);

   // Send the updated votes required amount to the calling cache_node.
   result = send_msg(buf, sizeof(CensusTemplate), MPI_UINT8_T, msg_info->src,
         GET_ACK, msg_info->comm, &request);
   MPI_ASSERT(result == MPI_SUCCESS);

   // Build push request message to send to all cache nodes which are to
   // participate in the voting.
   PushTemp push;
   PushTemplate *push_format = (PushTemplate *)buf;
   push.pack(push_format, get.job_num, get.job_node, msg_info->src, get.key);
   //push_format->job_num = job_num;
   //push_format->job_node = job_node;
   //push_format->key_size = key_size;
   //memcpy(push_format->key, key.c_str(), key_size);
   //push_format->cache_node = msg_info->src;

   MPI_Comm comm = node->job_to_comms[get.job_num].cache;
   // Ping all cache nodes which will participate in this quorum poll.
#ifdef DEBUG
   fprintf(stderr, "SIZE OF NODES ENTRY FOR KEY %s: %d\n", get.key.c_str(), nodes.size());
#endif
   for (auto const &entry : nodes) {
      if (entry.cache_node != msg_info->src) {
#ifdef DEBUG
         fprintf(stderr, "-- ENTRY -- \n\tjob_num: %d\n\tjob_node: %d\n\tcache_node: %d\n",
               entry.job_num, entry.job_node, entry.cache_node);
#endif
         result = send_msg(buf, sizeof(PushTemplate), MPI_UINT8_T,
               entry.cache_node, PUSH, comm, &request);
         MPI_ASSERT(result == MPI_SUCCESS);
      }
#ifdef DEBUG
      else {
         fprintf(stderr, "NOT SENDING PUSH REQUEST TO CACHENODE %d\n", msg_info->src);
      }
#endif
   }
}

// TODO: Shouldn't need this, but include for completeness
void SwingQuorum::handle_get_ack() {
   fprintf(stderr, "SwingQuorum handle_get_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

// TODO: Shouldn't need this, but include for completeness
void SwingQuorum::handle_get_local() {
   fprintf(stderr, "SwingQuorum handle_get_local not implemented!\n");
   MPI_ASSERT(FAILURE);
}

// TODO: Shouldn't need this, but include for completeness
void SwingQuorum::handle_get_local_ack() {
   fprintf(stderr, "SwingQuorum handle_get_local_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_push() {
   fprintf(stderr, "SwingQuorum handle_push not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_push_ack() {
   fprintf(stderr, "SwingQuorum handle_push_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_push_local() {
#ifdef DEBUG
   fprintf(stderr, "SwingQuorum handle_push_local\n");
   print_msg_info(&(node->msg_info));
#endif
   int result;

   recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, PUSH_LOCAL,
         msg_info->comm, status);

   PushTemplate *format = (PushTemplate *)buf;
   std::string key(format->key, format->key + format->key_size);

   JobNodeID id;
   id.job_num = format->job_num;
   id.job_node = format->job_node;
   id.cache_node = format->cache_node;

   // If the swing node did not have a mapping of the key to any nodes, add
   // the key mapping.
   if (node->key_to_nodes.count(key) == 0) {
#ifdef DEBUG
      fprintf(stderr, "----- Swing node %d did not have key %s in its key_to_nodes!\n",
            node->local_rank, key.c_str());
#endif
      std::vector<JobNodeID> v;
      v.push_back(id);
      node->key_to_nodes[key] = v;
   }
   // If the swing node had a mapping of the key to some nodes, ensure that the
   // new node that has the key is included in that mapping.
   else {
#ifdef DEBUG
      fprintf(stderr, "+++++ Swing node %d had key %s in its key_to_nodes!\n",
            node->local_rank, key.c_str());
#endif
      std::vector<JobNodeID> v = node->key_to_nodes[key];
      if (std::find(v.begin(), v.end(), id) == v.end()) {
         node->key_to_nodes[key].push_back(id);
      }

   }

#ifdef DEBUG
   std::vector<JobNodeID> v = node->key_to_nodes[key];
   for (std::vector<JobNodeID>::iterator it = v.begin();
         it != v.end(); ++it) {
      fprintf(stderr, "Swing node %d sees vector for key %s containing node %d of job %d with cache_node %d\n",
            node->local_rank, key.c_str(), it->job_node, it->job_num, it->cache_node);
   }
#endif

   MPI_Comm swing_comm = node->job_to_comms[id.job_num].swing;
   MPI_ASSERT(swing_comm != MPI_COMM_NULL);

   // If this message came from a cache node
   if (msg_info->comm != swing_comm) {
      // Forward this to all other swing nodes so everyone is current regarding
      // the key/value mapping.
      for (int i = 0; i < node->local_size; ++i) {
         // Don't want to send the PUT to this swing node!
         if (node->local_rank != i) {
#ifdef DEBUG
            fprintf(stderr, "SwingNode %d sending PUT for key %s to SwingNode %d!\n",
                  node->local_rank, key.c_str(), i);
#endif
            result = send_msg(buf, sizeof(PushTemplate), MPI_UINT8_T, i,
                  msg_info->tag, swing_comm, &request);
            MPI_ASSERT(result == MPI_SUCCESS);
         }
      }
   }

   // This is currently setup to immediately return without having the other
   // swing nodes propogate the updates to their child cache nodes.
   // Only reply if you are a swing node replying back to a cache node.
   if (msg_info->comm != swing_comm) {
      result = send_msg(buf, sizeof(PushAckTemplate), MPI_UINT8_T, msg_info->src,
            PUSH_LOCAL_ACK, msg_info->comm, &request);
      MPI_ASSERT(result == MPI_SUCCESS);
   }
}

void SwingQuorum::handle_push_local_ack() {
#ifdef DEBUG
   fprintf(stderr, "SwingQuorum handle_push_local_ack\n");
   print_msg_info(&(node->msg_info));
#endif
   int result;

   result = recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src,
                     PUSH_LOCAL_ACK, msg_info->comm, status);
   MPI_ASSERT(result == MPI_SUCCESS);

   fprintf(stderr, "SwingQuorum handle_put_local_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_pull() {
   fprintf(stderr, "SwingQuorum handle_pull not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_pull_ack() {
   fprintf(stderr, "SwingQuorum handle_pull_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_pull_local() {
   fprintf(stderr, "SwingQuorum handle_pull_local not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_pull_local_ack() {
   fprintf(stderr, "SwingQuorum handle_pull_local_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_scatter() {
   fprintf(stderr, "SwingQuorum handle_scatter not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_scatter_ack() {
   fprintf(stderr, "SwingQuorum handle_scatter_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_scatter_local() {
   fprintf(stderr, "SwingQuorum handle_scatter_local not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_scatter_local_ack() {
   fprintf(stderr, "SwingQuorum handle_scatter_local_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_gather() {
   fprintf(stderr, "SwingQuorum handle_gather not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_gather_ack() {
   fprintf(stderr, "SwingQuorum handle_gather_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_drop() {
   fprintf(stderr, "SwingQuorum handle_drop not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_drop_ack() {
   fprintf(stderr, "SwingQuorum handle_drop_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_drop_local() {
   fprintf(stderr, "SwingQuorum handle_drop_local not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_drop_local_ack() {
   fprintf(stderr, "SwingQuorum handle_drop_local_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_collect() {
   fprintf(stderr, "SwingQuorum handle_collect not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_collect_ack() {
   fprintf(stderr, "SwingQuorum handle_collect_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_get_owners() {
   fprintf(stderr, "SwingQuorum handle_get_owners not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void SwingQuorum::handle_get_owners_ack() {
   fprintf(stderr, "SwingQuorum handle_get_owners_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}
