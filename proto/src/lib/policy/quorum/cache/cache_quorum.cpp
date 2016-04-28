#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include "policy/quorum/cache/cache_quorum.h"
#include "utility/utils/utils.h"

CacheQuorum::CacheQuorum(Node *node) {
   MPI_ASSERT(node != NULL);
   this->node = node;
   this->buf = node->buf;
   this->msg_info = &(node->msg_info);
   this->status = &(node->status);
}

CacheQuorum::~CacheQuorum() {
}

void CacheQuorum::handle_put() {
#ifdef DEBUG
   printf("CACHE_NODE handle_put\n");
   print_msg_info(&(node->msg_info));
#endif
   fprintf(stderr, "CacheQuorum cache_handle_put not implemented!\n");
   MPI_ASSERT(FAILURE);
   /*
      int result;

      result = recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, PUT,
      msg_info->comm, status);
      MPI_ASSERT(result == MPI_SUCCESS);

   // Parse the Put message within buf, and add it to the cache.
   PutTemplate *format = (PutTemplate *)buf;
   std::string key(format->key, format->key + format->key_size);

   Parcel parcel;
   parcel.timestamp = format->timestamp;
   parcel.value = std::string(format->value, format->value + format->value_size);
   cache[key] = parcel;

   // Send msg to parent swing node.
   result = send_msg(buf, msg_info->count, MPI_UINT8_T, node->coord_rank, PUT,
   node->parent_comm, &request);
   MPI_ASSERT(result == MPI_SUCCESS);
   */
}

void CacheQuorum::handle_put_ack() {
#ifdef DEBUG
   printf("CACHE_NODE handle_put_ack\n");
   print_msg_info(&(node->msg_info));
#endif
   fprintf(stderr, "CacheQuorum cache_handle_put_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
   /*
      int result;

      result = recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, PUT_ACK,
      msg_info->comm, status);
      MPI_ASSERT(result == MPI_SUCCESS);

   // Parse the Put message within buf, and add it to the cache.
   PutAckTemplate *format = (PutAckTemplate *)buf;
   uint32_t job_num = format->job_num;
   uint32_t job_node = format->job_node;

   MPI_Comm job_comm = node->job_to_comms[job_num].job;

   // Send msg to job node informing it that the put is complete.
   result = send_msg(buf, sizeof(PutAckTemplate), MPI_UINT8_T, job_node, PUT_ACK,
   job_comm, &request);
   MPI_ASSERT(result == MPI_SUCCESS);
   */
}

void CacheQuorum::handle_put_local() {
#ifdef DEBUG
   printf("CACHE_NODE handle_put_local\n");
   print_msg_info(&(node->msg_info));
#endif
   int result;

   result = recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src,
         PUT_LOCAL, msg_info->comm, status);
   MPI_ASSERT(result == MPI_SUCCESS);

   // Parse the Put message within buf
   PutTemp put;
   PutTemplate *format = (PutTemplate *)buf;
   put.unpack(format);

   // Create a parcel to store in the cache
   Parcel parcel;
   parcel.timestamp = put.timestamp;
   parcel.value.assign(put.value);

   // If this is a new entry in the cache, notify the swing nodes so they know
   // about its existance for future requests.
   if (cache.count(put.key) == 0) {
      // Send msg to parent swing node.
      result = send_msg(buf, msg_info->count, MPI_UINT8_T, node->coord_rank,
            PUT_LOCAL, node->parent_comm, &request);
      MPI_ASSERT(result == MPI_SUCCESS);
   }
   // Otherwise, let the job node know the put_local is complete.
   else {
      // Grab the communicator associated with the target job node.
      MPI_Comm job_comm = node->job_to_comms[put.job_num].job;

      // Pack the Put Ack template
      PutAckTemp put_ack;
      PutAckTemplate *temp = (PutAckTemplate *)buf;
      put_ack.pack(temp, put.job_num, put.job_node, put.cache_node, put.key,
            result);

      // Send msg to job node informing it that the put_local is complete.
      result = send_msg(buf, sizeof(PutAckTemplate), MPI_UINT8_T,
            put.job_node, PUT_LOCAL_ACK, job_comm, &request);
      MPI_ASSERT(result == MPI_SUCCESS);
   }

   // Update the key locally.
   cache[put.key] = parcel;
}

void CacheQuorum::handle_put_local_ack() {
#ifdef DEBUG
   printf("CACHE_NODE handle_put_local_ack\n");
   print_msg_info(&(node->msg_info));
#endif
   int result;

   // Receive ack from swing node telling you it now knows you have a new key
   result = recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src,
         PUT_LOCAL_ACK, msg_info->comm, status);
   MPI_ASSERT(result == MPI_SUCCESS);

   // Parse the PutAck message within buf and determine which node it is for.
   PutAckTemplate *format = (PutAckTemplate *)buf;
   uint32_t job_num = format->job_num;
   uint32_t job_node = format->job_node;

   // Grab the communicator associated with the target job node.
   MPI_Comm job_comm = node->job_to_comms[job_num].job;

   // Send msg to job node informing it that the put_local is complete. Note
   // that we didn't set the result field of the message because the swing node
   // we passed this to would have done that.
   result = send_msg(buf, sizeof(PutAckTemplate), MPI_UINT8_T, job_node,
         PUT_LOCAL_ACK, job_comm, &request);
   MPI_ASSERT(result == MPI_SUCCESS);
}

void CacheQuorum::handle_get() {
#ifdef DEBUG
   printf("CACHE_NODE handle_get\n");
   print_msg_info(&(node->msg_info));
#endif
   fprintf(stderr, "CacheQuorum handle_get not implemented!\n");
   MPI_ASSERT(FAILURE);
   /*
      int result;

      result = recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, GET,
      msg_info->comm, status);
      MPI_ASSERT(result == MPI_SUCCESS);

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
   result = send_msg(buf, msg_info->count, MPI_UINT8_T, node->coord_rank, GET,
   node->parent_comm, &request);
   MPI_ASSERT(result == MPI_SUCCESS);
   */
}

void CacheQuorum::handle_get_ack() {
#ifdef DEBUG
   printf("CACHE_NODE handle_get_ack\n");
   print_msg_info(&(node->msg_info));
#endif
   fprintf(stderr, "CacheQuorum handle_get not implemented!\n");
   MPI_ASSERT(FAILURE);

   /*
      int result;

      result = recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, GET_ACK,
      msg_info->comm, status);
      MPI_ASSERT(result == MPI_SUCCESS);

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
   MPI_ASSERT(FAILURE);
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
GetReq new_vote;
GetAckTemplate *format = (GetAckTemplate *)buf;
new_vote.job_num = format->job_num;
new_vote.job_node = format->job_node;
new_vote.key =
std::string(format->key, format->key + format->key_size);

   // Match the GetReq object to an entry in pending_get_requests.
   it = std::find(pending_get_requests.begin(), pending_get_requests.end(),
   new_vote);

   if (it == pending_get_requests.end()) {
   // This should never happen so abort.
   MPI_ASSERT(FAILURE);
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
   printf("CacheNode %d - key %s - votes %d/%u\n", node->local_rank,
         it->key.c_str(), it->census.size(), it->votes_req);
}
#endif
*/
}

void CacheQuorum::handle_get_local() {
#ifdef DEBUG
   printf("CACHE_NODE handle_get_local\n");
   print_msg_info(&(node->msg_info));
#endif
   int result;
   uint8_t key_found = FAILURE;
   std::string value = "";
   uint64_t timestamp = 0;

   // Recv get_local request from the job node
   result = recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, GET_LOCAL,
         msg_info->comm, status);
   MPI_ASSERT(result == MPI_SUCCESS);

   // Parse the GetTemplate
   GetTemp get;
   GetTemplate *get_format = (GetTemplate *)buf;
   get.unpack(get_format);

   // Build the GetAckTemplate
   GetAckTemp get_ack;
   GetAckTemplate *get_ack_format = (GetAckTemplate *)node->buf;

   // Lookup the key locally
   if (cache.count(get.key) != 0) {
      Parcel parcel = cache[get.key];
      value.assign(parcel.value);
      timestamp = parcel.timestamp;
      key_found = SUCCESS;
   }

   // Pack the value string into message
   get_ack.pack(get_ack_format, get.job_num, get.job_node, get.key, value,
         timestamp, key_found);

   // Reply to the requesting job node.
   result = send_msg(node->buf, sizeof(GetAckTemplate), MPI_UINT8_T,
         get_ack.job_node, GET_LOCAL_ACK, msg_info->comm, &request);
   MPI_ASSERT(result == MPI_SUCCESS);
}

// TODO: Shouldn't need this, but include for completeness
void CacheQuorum::handle_get_local_ack() {
   fprintf(stderr, "CacheQuorum cache_node_handle_get_local_ack not implemented!\n");
   MPI_ASSERT(1 == 0);
}

// TODO: REWORK??
void CacheQuorum::handle_push() {
#ifdef DEBUG
   printf("CACHE_NODE handle_push\n");
   print_msg_info(&(node->msg_info));
#endif
   fprintf(stderr, "CacheQuorum handle_push not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void CacheQuorum::handle_push_ack() {
   fprintf(stderr, "CacheQuorum handle_push_local_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void CacheQuorum::handle_push_local() {
#ifdef DEBUG
   printf("CACHE_NODE handle_push_local\n");
   print_msg_info(&(node->msg_info));
#endif
   int result;

   // Recv push_local message from job node.
   result = recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src,
         PUSH_LOCAL, msg_info->comm, status);
   MPI_ASSERT(result == MPI_SUCCESS);

   // If this came from a cache node
   if (msg_info->comm == MPI_COMM_WORLD) {
      parse_and_save_push_local(buf, msg_info); 
   }
   // If this came from a job node
   else {
      push_to_target_node(buf, msg_info);
   }
}

void CacheQuorum::parse_and_save_push_local(uint8_t *buf, MsgInfo *msg_info) {
   int result;
   Parcel entry;
   uint8_t key_found;

   // This is a msg in Put format, parse it and add to cache
   PutTemp put;
   PutTemplate *put_format = (PutTemplate *)buf;
   put.unpack(put_format);

   // Parse out the contents of the new key/value.
   entry.value.assign(put.value);
   entry.timestamp = put.timestamp;

   // If you didn't have the key before, inform your coord swing node
   if (cache.count(put.key) == 0) {
      PushTemp push;
      PushTemplate *push_format = (PushTemplate *)buf;
      push.pack(push_format, put.job_num, put.job_node, put.cache_node,
            put.key);

      // Send message to coordinator swing node
      result = send_msg(buf, sizeof(PushTemplate), MPI_UINT8_T,
            node->coord_rank, PUSH_LOCAL, node->parent_comm, &request);
      MPI_ASSERT(result == MPI_SUCCESS);
   }
   // else respond back to the job node directly saying the push is complete
   else {
      key_found = SUCCESS;

      PushAckTemp push_ack;
      PushAckTemplate *push_ack_format = (PushAckTemplate *)buf;

      // Pack the rest of the push_ack.
      push_ack.pack(push_ack_format, put.job_num, put.job_node,
            put.cache_node, put.key, key_found);

      // Find the originator job's communicator.
      MPI_Comm job_comm = node->job_to_comms[put.job_num].job;

      // Reply back to original job node with outcome.
      result = send_msg(buf, sizeof(PushAckTemplate), MPI_UINT8_T,
            put.job_node, PUSH_LOCAL_ACK, job_comm, &request);
      MPI_ASSERT(result == MPI_SUCCESS);
   }

   // Add the key/value pair to this cache node.
   cache[put.key] = entry;
}

void CacheQuorum::push_to_target_node(uint8_t *buf, MsgInfo *msg_info) {
   int result;
   uint8_t key_found;

   // This is a message in Push format
   PushTemp push;
   PushTemplate *push_format = (PushTemplate *)buf;
   push.unpack(push_format);
   uint32_t target_node = push.cache_node;

   if (cache.count(push.key) == 0) {
      key_found = FAILURE;
   }
   else {
      key_found = SUCCESS;

      // If this message is telling this cache node to push a key/value pair to
      // another node, then push it!
      if (node->local_rank != target_node) {
         PutTemp put;
         PutTemplate *put_format = (PutTemplate *)buf;
         put.pack(put_format, push.job_num, push.job_node, target_node,
               push.key, cache[push.key].value, cache[push.key].timestamp);

         // Send the target cache node a Put message under a PUSH_LOCAL tag.
         result = send_msg(buf, sizeof(PutTemplate), MPI_UINT8_T,
               target_node, PUSH_LOCAL, MPI_COMM_WORLD, &request);
         MPI_ASSERT(result == MPI_SUCCESS);

         return;
      }
   }

   // If we got here then either we don't have the key, or we have the key and
   // the job node is trying to push the key to ourselves. In either case, we
   // need to ack back to the job node to let it know what is up.

   // Pack the rest of the push_ack.
   PushAckTemp push_ack;
   PushAckTemplate *push_ack_format = (PushAckTemplate *)buf;
   push_ack.pack(push_ack_format, push.job_num, push.job_node, target_node,
         push.key, key_found);

   // Find the originator job's communicator.
   MPI_Comm job_comm = node->job_to_comms[push.job_num].job;

   // Reply back to original job node with outcome.
   result = send_msg(buf, sizeof(PushAckTemplate), MPI_UINT8_T,
         push.job_node, PUSH_LOCAL_ACK, job_comm, &request);
   MPI_ASSERT(result == MPI_SUCCESS);
}

void CacheQuorum::handle_push_local_ack() {
#ifdef DEBUG
   printf("CACHE_NODE handle_push_local_ack\n");
   print_msg_info(&(node->msg_info));
#endif
   int result;

   result = recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src,
         PUSH_LOCAL_ACK, msg_info->comm, status);
   MPI_ASSERT(result == MPI_SUCCESS);

   PutAckTemplate *put_ack = (PutAckTemplate *)buf;

   put_ack->result = SUCCESS;
   uint32_t job_num = put_ack->job_num;
   uint32_t job_node = put_ack->job_node;

   MPI_Comm job_comm = node->job_to_comms[job_num].job;

   // Reply back to original job node with outcome.
   result = send_msg(buf, sizeof(PushAckTemplate), MPI_UINT8_T, job_node,
         PUSH_LOCAL_ACK, job_comm, &request);
   MPI_ASSERT(result == MPI_SUCCESS);
}

void CacheQuorum::handle_pull() {
   fprintf(stderr, "CacheQuorum handle_pull not implemented!\n");
   MPI_ASSERT(1 == 0);
}

void CacheQuorum::handle_pull_ack() {
   fprintf(stderr, "CacheQuorum handle_pull_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void CacheQuorum::handle_pull_local() {
   fprintf(stderr, "CacheQuorum handle_pull_local not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void CacheQuorum::handle_pull_local_ack() {
   fprintf(stderr, "CacheQuorum handle_pull_local_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void CacheQuorum::handle_scatter() {
   fprintf(stderr, "CacheQuorum handle_scatter not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void CacheQuorum::handle_scatter_ack() {
   fprintf(stderr, "CacheQuorum handle_scatter_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void CacheQuorum::handle_scatter_local() {
   fprintf(stderr, "CacheQuorum handle_scatter_local not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void CacheQuorum::handle_scatter_local_ack() {
   fprintf(stderr, "CacheQuorum handle_scatter_local_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void CacheQuorum::handle_gather() {
   fprintf(stderr, "CacheQuorum handle_gather not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void CacheQuorum::handle_gather_ack() {
   fprintf(stderr, "CacheQuorum handle_gather_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void CacheQuorum::handle_drop() {
   fprintf(stderr, "CacheQuorum handle_drop not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void CacheQuorum::handle_drop_ack() {
   fprintf(stderr, "CacheQuorum handle_drop_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void CacheQuorum::handle_drop_local() {
   fprintf(stderr, "CacheQuorum handle_drop_local not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void CacheQuorum::handle_drop_local_ack() {
   fprintf(stderr, "CacheQuorum handle_drop_local_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void CacheQuorum::handle_collect() {
   fprintf(stderr, "CacheQuorum handle_collect not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void CacheQuorum::handle_collect_ack() {
   fprintf(stderr, "CacheQuorum handle_collect_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void CacheQuorum::handle_get_owners() {
   fprintf(stderr, "CacheQuorum handle_get_owners not implemented!\n");
   MPI_ASSERT(FAILURE);
}

void CacheQuorum::handle_get_owners_ack() {
   fprintf(stderr, "CacheQuorum handle_get_owners_ack not implemented!\n");
   MPI_ASSERT(FAILURE);
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

   result = send_msg(node->buf, sizeof(GetAckTemplate), MPI_UINT8_T,
         format->job_node, GET_ACK, job_comm, &request);
   MPI_ASSERT(result == MPI_SUCCESS);
}
