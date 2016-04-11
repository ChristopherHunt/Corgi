#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include "cache_quorum.h"
#include "utils/utils.h"

CacheQuorum::CacheQuorum(Node *node) {
   this->node = node;
}

CacheQuorum::~CacheQuorum() {
}

void CacheQuorum::handle_put() {
#ifdef DEBUG
   printf("CACHE_NODE handle_put\n");
   print_msg_info(msg_info);
#endif
   int result;
   uint8_t *buf = node->buf;
   MsgInfo *msg_info = &(node->msg_info);
   MPI_Status *status = &(node->status);

   result = recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, PUT,
         msg_info->comm, status);
   ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));

   // Parse the Put message within buf, and add it to the cache.
   PutTemplate *format = (PutTemplate *)buf;
   std::string key(format->key, format->key + format->key_size);

   Parcel parcel;
   parcel.timestamp = format->timestamp;
   parcel.value = std::string(format->value, format->value + format->value_size);
   cache[key] = parcel;

   // Send msg to parent swing node.
   MPI_Request request;
   result = send_msg(buf, msg_info->count, MPI_UINT8_T, node->coord_rank, PUT,
         node->parent_comm, &request);
   ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));
}

void CacheQuorum::handle_put_ack() {
#ifdef DEBUG
   printf("CACHE_NODE handle_put_ack\n");
   print_msg_info(msg_info);
#endif

   int result;
   uint8_t *buf = node->buf;
   MsgInfo *msg_info = &(node->msg_info);
   MPI_Status *status = &(node->status);

   result = recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, PUT_ACK,
         msg_info->comm, status);
   ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));

   // Parse the Put message within buf, and add it to the cache.
   PutAckTemplate *format = (PutAckTemplate *)buf;
   uint32_t job_num = format->job_num;
   uint32_t job_node = format->job_node;

   MPI_Comm job_comm = node->job_to_comms[job_num].job;

   // Send msg to job node informing it that the put is complete.
   MPI_Request request;
   result = send_msg(buf, sizeof(PutAckTemplate), MPI_UINT8_T, job_node, PUT_ACK,
         job_comm, &request);
   ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));
}

void CacheQuorum::handle_put_local() {
#ifdef DEBUG
   printf("CACHE_NODE handle_put_local\n");
   print_msg_info(msg_info);
#endif
   int result;
   uint8_t *buf = node->buf;
   MsgInfo *msg_info = &(node->msg_info);
   MPI_Status *status = &(node->status);

   result = recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, PUT_LOCAL,
         msg_info->comm, status);
   ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));

   // Parse the Put message within buf, and add it to the cache.
   PutTemplate *format = (PutTemplate *)buf;
   std::string key(format->key, format->key + format->key_size);

   Parcel parcel;
   parcel.timestamp = format->timestamp;
   parcel.value = std::string(format->value, format->value + format->value_size);

   // If this is a new entry in the cache, notify the swing nodes so they know
   // about its existance for future requests.
   if (cache.count(key) == 0) {
      // Send msg to parent swing node.
      MPI_Request request;
      result = send_msg(buf, msg_info->count, MPI_UINT8_T, node->coord_rank,
            PUT_LOCAL, node->parent_comm, &request);
      ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));
   }

   // Update the key locally.
   cache[key] = parcel;
}

void CacheQuorum::handle_put_local_ack() {
#ifdef DEBUG
   printf("CACHE_NODE handle_put_local_ack\n");
   print_msg_info(msg_info);
#endif
   int result;
   uint8_t *buf = node->buf;
   MsgInfo *msg_info = &(node->msg_info);
   MPI_Status *status = &(node->status);

   result = recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src,
                     PUT_LOCAL_ACK, msg_info->comm, status);
   ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));

   // Parse the PutAck message within buf and determine which node it is for.
   PutAckTemplate *format = (PutAckTemplate *)buf;
   uint32_t job_num = format->job_num;
   uint32_t job_node = format->job_node;

   // Grab the communicator associated with the target job node.
   MPI_Comm job_comm = node->job_to_comms[job_num].job;

   // Send msg to job node informing it that the put_local is complete.
   MPI_Request request;
   result= send_msg(buf, sizeof(PutAckTemplate), MPI_UINT8_T, job_node,
         PUT_LOCAL_ACK, job_comm, &request);
   ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));
}

void CacheQuorum::handle_get() {
#ifdef DEBUG
   printf("CACHE_NODE handle_get\n");
   print_msg_info(msg_info);
#endif
   int result;
   uint8_t *buf = node->buf;
   MsgInfo *msg_info = &(node->msg_info);
   MPI_Status *status = &(node->status);

   result = recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, GET,
         msg_info->comm, status);
   ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));

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
   result = send_msg(buf, msg_info->count, MPI_UINT8_T, node->coord_rank, GET,
         node->parent_comm, &request);
   ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));
}

void CacheQuorum::handle_get_ack() {
#ifdef DEBUG
   printf("CACHE_NODE handle_get_ack\n");
   print_msg_info(msg_info);
#endif
   int result;
   uint8_t *buf = node->buf;
   MsgInfo *msg_info = &(node->msg_info);
   MPI_Status *status = &(node->status);

   result = recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, GET_ACK,
         msg_info->comm, status);
   ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));

   // Iterator to find the GetReq that matches this get_ack.
   std::vector<GetReq>::iterator it;

   // If this is a message from this cache_node's coordinator swing_node
   if (msg_info->comm == node->parent_comm && msg_info->src == node->coord_rank) {
#ifdef DEBUG
      printf("CacheNode %d got CensusTemplate from Swing Node!\n",\
         node->local_rank);
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
         ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
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
         ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
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

void CacheQuorum::handle_get_local() {
#ifdef DEBUG
   printf("CACHE_NODE handle_get_local\n");
   print_msg_info(msg_info);
#endif
   int result;
   uint8_t *buf = node->buf;
   MsgInfo *msg_info = &(node->msg_info);
   MPI_Status *status = &(node->status);

   result = recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, GET_LOCAL,
         msg_info->comm, status);
   ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));

   // Parse the GetTemplate
   GetTemplate *get_format = (GetTemplate *)buf;
   uint32_t job_num = get_format->job_num;
   uint32_t job_node = get_format->job_node;
   uint32_t key_size = get_format->key_size;
   std::string key(get_format->key, get_format->key + key_size);

   // Lookup the key locally
   std::string value = cache[key].value;

   // Build the GetAckTemplate
   // I know some of these are just straight reassignments, but I am being
   // thorough in case the templates change in the future.
   GetAckTemplate *get_ack_format = (GetAckTemplate *)node->buf;
   get_ack_format->job_num = job_num;
   get_ack_format->job_node = job_node;
   get_ack_format->key_size = key_size;
   memcpy(get_ack_format->key, key.c_str(), key_size);
   get_ack_format->value_size = value.size();
   memcpy(get_ack_format->value, value.c_str(), value.size());

   // Reply
   MPI_Request request;
   result = send_msg(node->buf, sizeof(GetAckTemplate), MPI_UINT8_T,
         job_node, GET_LOCAL_ACK, msg_info->comm, &request);
   ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));
}

// TODO: Shouldn't need this, but include for completeness
void CacheQuorum::handle_get_local_ack() {
   fprintf(stderr, "CacheQuorum cache_node_handle_get_local_ack not implemented!\n");
   ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
}

void CacheQuorum::handle_push() {
#ifdef DEBUG
   printf("CACHE_NODE handle_push\n");
   print_msg_info(msg_info);
#endif

   int result;
   uint8_t *buf = node->buf;
   MsgInfo *msg_info = &(node->msg_info);
   MPI_Status *status = &(node->status);

   result = recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, PUSH,
         msg_info->comm, status);
   ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));

   // Parse the message.
   ForwardTemplate *push_format = (ForwardTemplate *)buf;
   uint32_t job_num = push_format->job_num;
   uint32_t job_node = push_format->job_node;
   uint32_t cache_node = push_format->cache_node;
   std::string key(push_format->key, push_format->key +
         push_format->key_size);
   Parcel parcel;

   // Get the value corresponding to the specified key.
   if (cache.count(key) != 0) {
      parcel = cache[key];
   }
   else {
      ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
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
   result = send_msg(buf, sizeof(GetAckTemplate), MPI_UINT8_T, cache_node,
         GET_ACK, MPI_COMM_WORLD, &request);
   ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));
}

void CacheQuorum::handle_push_ack() {
   fprintf(stderr, "CacheQuorum handle_push_ack not implemented!\n");
   ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
}

void CacheQuorum::handle_pull() {
   fprintf(stderr, "CacheQuorum handle_pull not implemented!\n");
   ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
}

void CacheQuorum::handle_pull_ack() {
   fprintf(stderr, "CacheQuorum handle_pull_ack not implemented!\n");
   ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
}

void CacheQuorum::handle_scatter() {
   fprintf(stderr, "CacheQuorum handle_scatter not implemented!\n");
   ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
}

void CacheQuorum::handle_scatter_ack() {
   fprintf(stderr, "CacheQuorum handle_scatter_ack not implemented!\n");
   ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
}

void CacheQuorum::handle_gather() {
   fprintf(stderr, "CacheQuorum handle_gather not implemented!\n");
   ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
}

void CacheQuorum::handle_gather_ack() {
   fprintf(stderr, "CacheQuorum handle_gather_ack not implemented!\n");
   ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
}

void CacheQuorum::handle_drop() {
   fprintf(stderr, "CacheQuorum handle_drop not implemented!\n");
   ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
}

void CacheQuorum::handle_drop_ack() {
   fprintf(stderr, "CacheQuorum handle_drop_ack not implemented!\n");
   ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
}

void CacheQuorum::handle_collect() {
   fprintf(stderr, "CacheQuorum handle_collect not implemented!\n");
   ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
}

void CacheQuorum::handle_collect_ack() {
   fprintf(stderr, "CacheQuorum handle_collect_ack not implemented!\n");
   ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
}

void CacheQuorum::handle_get_owners() {
   fprintf(stderr, "CacheQuorum handle_get_owners not implemented!\n");
   ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
}

void CacheQuorum::handle_get_owners_ack() {
   fprintf(stderr, "CacheQuorum handle_get_owners_ack not implemented!\n");
   ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
}

void CacheQuorum::send_job_node_get_ack(std::vector<GetReq>::iterator get_req_it) {
#ifdef DEBUG
   printf("SEND_JOB_NODE_GET_ACK!\n");
#endif

   int result;

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

   result = send_msg(node->buf, sizeof(GetAckTemplate), MPI_UINT8_T,
         format->job_node, GET_ACK, job_comm, &request);
   ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));
}
