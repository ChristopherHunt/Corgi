#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include "quorum.h"

Quorum::Quorum(Node *node, NodeType node_type) {
   this->node_type = node_type;
   this->node = node;
}

Quorum::~Quorum() {
}

void Quorum::swing_node_handle_put(uint8_t *buf, MsgInfo *msg_info) {
#ifdef DEBUG
   printf("SWING_NODE handle_put\n");
   print_msg_info(msg_info);
#endif

   PutTemplate *format = (PutTemplate *)buf;
   std::string key(format->key, format->key + format->key_size);

   JobNodeID id;
   id.job_num = format->job_num;
   id.job_node = format->job_node;
   id.cache_node = format->cache_node;

   // If the swing node did not have a mapping of the key to any nodes, add
   // the key mapping.
   if (node->key_to_nodes.count(key) == 0) {
#ifdef DEBUG
      printf("----- Swing node %d did not have key %s in its key_to_nodes!\n",
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
      printf("+++++ Swing node %d had key %s in its key_to_nodes!\n",
            node->local_rank, key.c_str());
#endif
      std::vector<JobNodeID> v = node->key_to_nodes[key];
      if (std::find(v.begin(), v.end(), id) == v.end()) {
         node->key_to_nodes[key].push_back(id);
      }

   }

#ifdef DEBUG
   // TODO: REMOVE
   std::vector<JobNodeID> v = node->key_to_nodes[key];
   for (std::vector<JobNodeID>::iterator it = v.begin();
         it != v.end(); ++it) {
      printf("Swing node %d sees vector for key %s containing node %d of job %d with cache_node %d\n",
            node->local_rank, key.c_str(), it->job_node, it->job_num, it->cache_node);
   }
   //
#endif

   MPI_Request request;
   MPI_Comm swing_comm = node->job_to_comms[id.job_num].swing;

   if (msg_info->comm != swing_comm) {
      // Forward this to all other swing nodes so everyone is current regarding
      // the key/value mapping.
      for (int i = 0; i < node->local_size; ++i) {
         // Don't want to send the put to this swing node!
         if (node->local_rank != i) {
#ifdef DEBUG
            printf("SwingNode %d sending PUT for key %s to SwingNode %d!\n",
                  node->local_rank, key.c_str(), i);
#endif
            send_msg(buf, sizeof(PutTemplate), MPI_UINT8_T, i, PUT,
                  swing_comm, &request);
         }
      }
   }

   // This is currently setup to immediately return without having the other
   // swing nodes propogate the updates to their child cache nodes. So this is
   // a W = 1, R = N policy currently. Update this later to be more flexible.
   send_msg(buf, sizeof(PutAckTemplate), MPI_UINT8_T, msg_info->src, PUT_ACK,
         msg_info->comm, &request);
}

void Quorum::cache_node_handle_put(uint8_t *buf, MsgInfo *msg_info) {
#ifdef DEBUG
   printf("CACHE_NODE handle_put\n");
   print_msg_info(msg_info);
#endif

   // Parse the Put message within buf, and add it to the cache.
   PutTemplate *format = (PutTemplate *)buf;
   std::string key(format->key, format->key + format->key_size);

   Parcel parcel;
   parcel.timestamp = format->timestamp;
   parcel.value = std::string(format->value, format->value + format->value_size);
   cache[key] = parcel;

   // Send msg to parent swing node.
   MPI_Request request;
   send_msg(buf, msg_info->count, MPI_UINT8_T, node->coord_rank, PUT,
         node->parent_comm, &request);
}

void Quorum::swing_node_handle_put_ack(uint8_t *buf, MsgInfo *msg_info) {
#ifdef DEBUG
   printf("SWING_NODE handle_put_ack\n");
#endif
}

void Quorum::cache_node_handle_put_ack(uint8_t *buf, MsgInfo *msg_info) {
#ifdef DEBUG
   printf("CACHE_NODE handle_put_ack\n");
   print_msg_info(msg_info);
#endif

   // Parse the Put message within buf, and add it to the cache.
   PutAckTemplate *format = (PutAckTemplate *)buf;
   uint32_t job_num = format->job_num;
   uint32_t job_node = format->job_node;

   MPI_Comm job_comm = node->job_to_comms[job_num].job;

   // Send msg to job node informing it that the put is complete.
   MPI_Request request;
   send_msg(buf, sizeof(PutAckTemplate), MPI_UINT8_T, job_node, PUT_ACK, job_comm,
         &request);
}

void Quorum::swing_node_handle_get(uint8_t *buf, MsgInfo *msg_info) {
#ifdef DEBUG
   printf("SWING_NODE handle_get\n");
   print_msg_info(msg_info);
#endif

   GetTemplate *format = (GetTemplate *)buf;
   uint32_t job_num = format->job_num;
   uint32_t job_node = format->job_node;
   uint32_t key_size = format->key_size;
   std::string key(format->key, format->key + key_size);
   uint64_t timestamp = format->timestamp;

   // Grab the list of nodes which have a value associated with the target key.
   std::vector<JobNodeID> nodes = node->key_to_nodes[key];

   // Build the new message to send to the calling cache node (a lot of this is
   // reassignment but its safer for now to do it this way, we can strip out
   // the reassihnmnet to the buffer later). The main point of this message is
   // to tell the calling cache node how many of its peers need to vote on this
   // GetReq before it can decide it has enough data to choose the correct
   // value based on timestamps.
   CensusTemplate *census_format = (CensusTemplate *)buf;
   census_format->job_num = job_num;
   census_format->job_node = job_node;
   census_format->key_size = key_size;
   memcpy(census_format->key, key.c_str(), key_size);

   // If nobody has the key/value, then set votes to 0 so the caller knows this
   // key does not exist in the cache. 
   if (nodes.size() == 0) {
      census_format->votes_req = 0;
   }
   // Otherwise, set the votes required to the proper number of nodes.
   // TODO: Make this not just W = 1, R = N! Add flexibility later.
   else {
      census_format->votes_req = nodes.size();
   }

   // Send the updated votes required amount to the calling cache_node.
   MPI_Request request;
   send_msg(buf, sizeof(CensusTemplate), MPI_UINT8_T, msg_info->src, GET_ACK,
         msg_info->comm, &request);

   // Build forward request message to send to all cache nodes which are to
   // participate in the voting.
   ForwardTemplate *forward_format = (ForwardTemplate *)buf;
   forward_format->job_num = job_num;
   forward_format->job_node = job_node;
   forward_format->key_size = key_size;
   memcpy(forward_format->key, key.c_str(), key_size);
   forward_format->cache_node = msg_info->src;

   MPI_Comm comm = node->job_to_comms[job_num].cache;
   // Ping all cache nodes which will participate in this quorum poll.
#ifdef DEBUG
   printf("SIZE OF NODES ENTRY FOR KEY %s: %d\n", key.c_str(), nodes.size());
#endif
   for (auto const &entry : nodes) {
      if (entry.cache_node != msg_info->src) {
#ifdef DEBUG
         printf("-- ENTRY -- \n\tjob_num: %d\n\tjob_node: %d\n\tcache_node: %d\n",
               entry.job_num, entry.job_node, entry.cache_node);
#endif
         send_msg(buf, sizeof(ForwardTemplate), MPI_UINT8_T, entry.cache_node,
               FORWARD, comm, &request);
      }
#ifdef DEBUG
      else {
         printf("NOT SENDING FORWARD REQUEST TO CACHENODE %d\n", msg_info->src);
      }
#endif
   }
}

void Quorum::cache_node_handle_get(uint8_t *buf, MsgInfo *msg_info) {
#ifdef DEBUG
   printf("CACHE_NODE handle_get\n");
   print_msg_info(msg_info);
#endif

   GetTemplate *format = (GetTemplate *)buf;

   // Build a GetReq for this key/value lookup so the cache nodes can vote as a
   // whole and achieve consistency.
   GetReq get_req;
   get_req.job_num = format->job_num;
   get_req.job_node = format->job_node;
   get_req.key = std::string(format->key, format->key + format->key_size);

   // Set max voter size and upate later when the cache_node hears back from 
   // its coord_swing node.
   get_req.votes_req = node->local_size;
   get_req.census = std::vector<Parcel>();

   // If this cache node has the key, put it inside the GetReq's census vector.
   if (cache.count(get_req.key) != 0) {
      get_req.census.push_back(cache[get_req.key]);
   }

   // Add the GetReq to the vector of pending requests for this cache node.
   pending_get_requests.push_back(get_req);

   // Pass on the GET message to this cache node's coordinator swing node.
   MPI_Request request;
   send_msg(buf, msg_info->count, MPI_UINT8_T, node->coord_rank, GET,
         node->parent_comm, &request);
}

void Quorum::swing_node_handle_get_ack(uint8_t *buf, MsgInfo *msg_info) {
#ifdef DEBUG
   printf("SWING_NODE handle_get_ack\n");
   print_msg_info(msg_info);
#endif
}

void Quorum::cache_node_handle_get_ack(uint8_t *buf, MsgInfo *msg_info) {
#ifdef DEBUG
   printf("CACHE_NODE handle_get_ack\n");
   print_msg_info(msg_info);
#endif

   // Iterator to find the GetReq that matches this get_ack.
   std::vector<GetReq>::iterator it;

   // If this is a message from this cache_node's coordinator swing_node
   if (msg_info->comm == node->parent_comm && msg_info->src == node->coord_rank) {
#ifdef DEBUG
      printf("CacheNode %d got CensusTemplate from Swing Node!\n", node->local_rank);
#endif
      // Build out the match for the GetReq entry from the message.
      CensusTemplate *format = (CensusTemplate *)buf;
      GetReq updated_vote;
      updated_vote.job_num = format->job_num;
      updated_vote.job_node = format->job_node;
      updated_vote.key =
         std::string(format->key, format->key + format->key_size);

      // Match the GetReq object to an entry in pending_get_requests.
      it = std::find(pending_get_requests.begin(), pending_get_requests.end(),
            updated_vote);

      if (it == pending_get_requests.end()) {
         // This should never happen so abort.
         ASSERT_TRUE(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
      }
      else {
         // Update the votes_required
         it->votes_req = format->votes_req;
      }
   }
   else {
#ifdef DEBUG
      printf("CacheNode %d got vote from CacheNode %d!\n", node->local_rank,
            msg_info->src);
#endif
      GetAckTemplate *format = (GetAckTemplate *)buf;
      GetReq new_vote;
      new_vote.job_num = format->job_num;
      new_vote.job_node = format->job_node;
      new_vote.key =
         std::string(format->key, format->key + format->key_size);

      // Match the GetReq object to an entry in pending_get_requests.
      it = std::find(pending_get_requests.begin(), pending_get_requests.end(),
            new_vote);

      if (it == pending_get_requests.end()) {
         // This should never happen so abort.
         ASSERT_TRUE(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
      }
      else {
         // Add the new value to the vector of votes.
         Parcel vote;
         vote.value = std::string (format->value,
               format->value + format->value_size);
         vote.timestamp = format->timestamp;
         it->census.push_back(vote);
      }
   }

   if (it->votes_req == it->census.size()) {
#ifdef DEBUG
      printf("CacheNode %d - key %s - received all %d votes!\n",
            node->local_rank, it->key.c_str(), it->census.size());
#endif
      send_job_node_get_ack(it);
      pending_get_requests.erase(it);
   }
#ifdef DEBUG
   else {
      printf("CacheNode %d - key %s - votes %d/%d\n", node->local_rank,
            it->key.c_str(), it->census.size(), it->votes_req);
   }
#endif
}

void Quorum::swing_node_handle_forward(uint8_t *buf, MsgInfo *msg_info) {
#ifdef DEBUG
   printf("SWING_NODE handle_forward\n");
   print_msg_info(msg_info);
#endif
}

void Quorum::cache_node_handle_forward(uint8_t *buf, MsgInfo *msg_info) {
#ifdef DEBUG
   printf("CACHE_NODE handle_forward\n");
   print_msg_info(msg_info);
#endif

   // Parse the message.
   ForwardTemplate *forward_format = (ForwardTemplate *)buf;
   uint32_t job_num = forward_format->job_num;
   uint32_t job_node = forward_format->job_node;
   uint32_t cache_node = forward_format->cache_node;
   std::string key(forward_format->key, forward_format->key +
         forward_format->key_size);
   Parcel parcel;

   // Get the value corresponding to the specified key.
   if (cache.count(key) != 0) {
      parcel = cache[key];
   }
   else {
      ASSERT_TRUE(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
   }
   std::string value = parcel.value;

   // Build out the acknowledge response to the initial cache node that
   // initiated the get request for this key/value.
   GetAckTemplate *get_format = (GetAckTemplate *)buf;
   get_format->job_num = job_num;
   get_format->job_node = job_node;
   get_format->key_size = key.size();
   memcpy(get_format->key, key.c_str(), key.size());
   get_format->value_size = value.size();
   memcpy(get_format->value, value.c_str(), value.size());
   get_format->timestamp = parcel.timestamp;

   MPI_Request request;
   send_msg(buf, sizeof(GetAckTemplate), MPI_UINT8_T, cache_node, GET_ACK,
         MPI_COMM_WORLD, &request);
}

void Quorum::swing_node_handle_forward_ack(uint8_t *buf, MsgInfo *msg_info) {
#ifdef DEBUG
   printf("SWING_NODE handle_forward_ack\n");
   print_msg_info(msg_info);
#endif
}

void Quorum::cache_node_handle_forward_ack(uint8_t *buf, MsgInfo *msg_info) {
#ifdef DEBUG
   printf("CACHE_NODE handle_forward_ack\n");
   print_msg_info(msg_info);
#endif
}

void Quorum::send_job_node_get_ack(std::vector<GetReq>::iterator get_req_it) {
#ifdef DEBUG
   printf("SEND_JOB_NODE_GET_ACK!\n");
#endif
   std::vector<Parcel> census = get_req_it->census;
   Parcel winning_vote;
   winning_vote.timestamp = 0;

   for (std::vector<Parcel>::iterator it = census.begin();
         it != census.end(); ++it) {
      if (it->timestamp > winning_vote.timestamp) {
         winning_vote.timestamp = it->timestamp;
         winning_vote.value = it->value;
      }
   }

   GetAckTemplate *format = (GetAckTemplate *)node->buf;
   format->job_num = get_req_it->job_num;
   format->job_node = get_req_it->job_node;
   format->key_size = get_req_it->key.size();
   memcpy(format->key, get_req_it->key.c_str(), format->key_size);
   format->value_size = winning_vote.value.size();
   memcpy(format->value, winning_vote.value.c_str(), winning_vote.value.size());
   format->timestamp = winning_vote.timestamp;

   MPI_Comm job_comm = node->job_to_comms[format->job_num].job;
   MPI_Request request;

   send_msg(node->buf, sizeof(GetAckTemplate), MPI_UINT8_T, format->job_node,
         GET_ACK, job_comm, &request);
}

void Quorum::handle_put() {
   uint8_t *buf = node->buf;
   MsgInfo *msg_info = &(node->msg_info);
   MPI_Status *status = &(node->status);

   recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, PUT,
         msg_info->comm, status);

   switch (node_type) {
      case SWING:
         swing_node_handle_put(buf, msg_info);
         break;

      case CACHE:
         cache_node_handle_put(buf, msg_info);
         break;

      default:
         ASSERT_TRUE(1 == 0, MPI_Abort(1, MPI_COMM_WORLD));
         break;
   }
}

void Quorum::handle_put_ack() {
   uint8_t *buf = node->buf;
   MsgInfo *msg_info = &(node->msg_info);
   MPI_Status *status = &(node->status);

   recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, PUT_ACK,
         msg_info->comm, status);

   switch (node_type) {
      case SWING:
         swing_node_handle_put_ack(buf, msg_info);
         break;

      case CACHE:
         cache_node_handle_put_ack(buf, msg_info);
         break;

      default:
         ASSERT_TRUE(1 == 0, MPI_Abort(1, MPI_COMM_WORLD));
         break;
   }
}

void Quorum::handle_get() {
   uint8_t *buf = node->buf;
   MsgInfo *msg_info = &(node->msg_info);
   MPI_Status *status = &(node->status);

   recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, GET,
         msg_info->comm, status);

   switch (node_type) {
      case SWING:
         swing_node_handle_get(buf, msg_info);
         break;

      case CACHE:
         cache_node_handle_get(buf, msg_info);
         break;

      default:
         ASSERT_TRUE(1 == 0, MPI_Abort(1, MPI_COMM_WORLD));
         break;
   }
}

void Quorum::handle_get_ack() {
   uint8_t *buf = node->buf;
   MsgInfo *msg_info = &(node->msg_info);
   MPI_Status *status = &(node->status);

   recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, GET_ACK,
         msg_info->comm, status);

   switch (node_type) {
      case SWING:
         swing_node_handle_get_ack(buf, msg_info);
         break;

      case CACHE:
         cache_node_handle_get_ack(buf, msg_info);
         break;

      default:
         ASSERT_TRUE(1 == 0, MPI_Abort(1, MPI_COMM_WORLD));
         break;
   }
}

void Quorum::handle_forward() {
   uint8_t *buf = node->buf;
   MsgInfo *msg_info = &(node->msg_info);
   MPI_Status *status = &(node->status);

   recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, FORWARD,
         msg_info->comm, status);

   switch (node_type) {
      case SWING:
         swing_node_handle_forward(buf, msg_info);
         break;

      case CACHE:
         cache_node_handle_forward(buf, msg_info);
         break;

      default:
         ASSERT_TRUE(1 == 0, MPI_Abort(1, MPI_COMM_WORLD));
         break;
   }
}

void Quorum::handle_forward_ack() {
   uint8_t *buf = node->buf;
   MsgInfo *msg_info = &(node->msg_info);
   MPI_Status *status = &(node->status);

   recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, GET_ACK,
         msg_info->comm, status);

   switch (node_type) {
      case SWING:
         swing_node_handle_forward_ack(buf, msg_info);
         break;

      case CACHE:
         cache_node_handle_forward_ack(buf, msg_info);
         break;

      default:
         ASSERT_TRUE(1 == 0, MPI_Abort(1, MPI_COMM_WORLD));
         break;
   }
}
