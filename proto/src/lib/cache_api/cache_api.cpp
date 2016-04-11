#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include "network/network.h"
#include "utils/utils.h"
#include "cache_api.h"

Cache::Cache(int *argc_ptr, char ***argv_ptr) {
   ASSERT(argc_ptr != NULL, MPI_Abort(MPI_COMM_WORLD, 1));
   ASSERT(argv_ptr != NULL, MPI_Abort(MPI_COMM_WORLD, 1));

   allocate();
   orient(argc_ptr, argv_ptr); 
}

void Cache::allocate() {
   buf = (uint8_t *)calloc(INITIAL_BUF_SIZE, sizeof(uint8_t)); 
   ASSERT(buf != NULL, MPI_Abort(MPI_COMM_WORLD, 1));
}

Cache::~Cache() {
   free(buf);
}

bool Cache::put_local(const std::string& key, const std::string& value) {
   return handle_put(key, value, PUT_LOCAL);
}

// TODO: Handle put failures from cache node!
bool Cache::put(const std::string& key, const std::string& value) {
   return handle_put(key, value, PUT);
}

bool Cache::get(const std::string& key, std::string& value) {
   return handle_get(key, value, GET);
}

bool Cache::get_local(const std::string& key, std::string& value) {
   return handle_get(key, value, GET_LOCAL);
}

bool Cache::push(const std::string& key, uint32_t node_id) {
   fprintf(stderr, "push not implemented!\n");
   ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
   return false;
}

bool Cache::pull(const std::string& key, uint32_t node_id) {
   fprintf(stderr, "pull not implemented!\n");
   ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
   return false;
}

bool Cache::scatter(const std::string& key,
      const std::vector<uint32_t>& node_ids) {

   fprintf(stderr, "scatter not implemented!\n");
   ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
   return false;
}

bool Cache::gather(const std::string& key,
      const std::vector<uint32_t>& node_ids) {

   fprintf(stderr, "gather not implemented!\n");
   ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
   return false;
}

bool Cache::drop(const std::string& key) {
   fprintf(stderr, "drop not implemented!\n");
   ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
   return false;
}

bool Cache::collect(const std::string& key) {
   fprintf(stderr, "collect not implemented!\n");
   ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
   return false;
}

void Cache::get_owners(const std::string& key, std::vector<uint32_t>& owners) {
   fprintf(stderr, "get_owners not implemented!\n");
   ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
}

void Cache::orient(int *argc_ptr, char ***argv_ptr) {
   ASSERT(argc_ptr != NULL, MPI_Abort(MPI_COMM_WORLD, 1));
   ASSERT(argv_ptr != NULL, MPI_Abort(MPI_COMM_WORLD, 1));

   int argc = *argc_ptr;
   char **argv = *argv_ptr;

   ASSERT(argc >= 3, MPI_Abort(1, MPI_COMM_WORLD));

#ifdef DEBUG
   // Print the argv list for reference
   printf("job_node argc: %d\n", argc);
   for (int i = 0; i < argc; ++i) {
      printf("argv[%d]: %s\n", i, argv[i]);
   }
   //
#endif

   // Get the job_num for this job. 
   char *endptr;
   job_num = strtol(argv[1], &endptr, 10);

   // Grab the job to cache node pairings list. 
   std::string mapping(argv[2]);
   //replace_commas(mapping);
   std::vector<uint32_t> map_vec;
   stringlist_to_vector(map_vec, mapping);

   // Update argc and argv so things are transparent to the caller.
   // TODO: Ensure this is working properly
   argc_ptr -= 1;
   std::string exec_name(argv[0]);
   memcpy(argv[2], exec_name.c_str(), exec_name.size());
   *argv_ptr = *(argv_ptr + 2);

   // Get details on the world this node lives in.
   MPI_Comm_size(MPI_COMM_WORLD, &local_size);
   MPI_Comm_rank(MPI_COMM_WORLD, &local_rank);

   MPI_Comm_get_parent(&parent_comm);
   MPI_Comm_remote_size(parent_comm, &parent_size);
   MPI_Comm_rank(parent_comm, &parent_rank);

   // Get coordinator cache node's rank for this job node.
   coord_rank = map_vec[local_rank];

#ifdef DEBUG
   printf("Job node: local rank - %d/%d parent rank - %d/%d\n", local_rank,
         local_size, parent_rank, parent_size);

   printf("Job Num: %d Job node %d: team cache node: %d\n", job_num, local_rank,
         coord_rank);
#endif
}

bool Cache::handle_put(const std::string& key, const std::string& value,
      MsgTag tag) {

   ASSERT(tag == PUT || tag == PUT_LOCAL, MPI_Abort(MPI_COMM_WORLD, 1));

   int32_t result;
   MPI_Request request;
   MPI_Status status;
   MsgTag recv_tag = tag == PUT ? PUT_ACK : PUT_LOCAL_ACK;

   // Pack the put message into buf prior to sending.
   pack_put(key, value);

#ifdef DEBUG
   if (tag == PUT) {
      printf("Job %d Rank %d calling put on %s/%s!\n", job_num, local_rank,
            key.c_str(), value.c_str());
   }
   else {
      printf("Job %d Rank %d calling put_local on %s/%s!\n", job_num,
            local_rank, key.c_str(), value.c_str());
   }
#endif

   result = send_msg(buf, sizeof(PutTemplate), MPI_UINT8_T, coord_rank, tag,
         parent_comm, &request);

   if (result != MPI_SUCCESS) {
      return false;
   }

   // Assuming this cache is not doing any non-blocking calls, and as a result
   // this PUT_ACK/PUT_LOCAL_ACK is guaranteed to be the ack for the send we
   // just made. This assumption would not hold if this cache did non-blocking
   // IO.
   result = recv_msg(buf, sizeof(PutAckTemplate), MPI_UINT8_T, coord_rank,
         recv_tag, parent_comm, &status);

   return result == MPI_SUCCESS ? true : false;
}

bool Cache::handle_get(const std::string& key, std::string& value, MsgTag tag) {
   int result;

   ASSERT(tag == GET || tag == GET_LOCAL, MPI_Abort(MPI_COMM_WORLD, 1));

   MPI_Request request;
   MPI_Status status;
   MsgTag recv_tag = tag == GET ? GET_ACK : GET_LOCAL_ACK;

   // Pack the get message into buf prior to sending.
   pack_get(key);

#ifdef DEBUG
   if (recv_tag == GET) {
      printf("Job %d Rank %d calling get on key %s!\n", job_num, local_rank,
            key.c_str());
   }
   else {
      printf("Job %d Rank %d calling get_local on key %s!\n", job_num,
            local_rank, key.c_str());
   }
#endif

   result = send_msg(buf, sizeof(GetTemplate), MPI_UINT8_T, coord_rank,
         tag, parent_comm, &request);

   if (result != MPI_SUCCESS) {
      return false;
   }

   result = recv_msg(buf, sizeof(GetAckTemplate), MPI_UINT8_T, coord_rank,
         recv_tag, parent_comm, &status);

   if (result != MPI_SUCCESS) {
      return false;
   }

   GetAckTemplate *temp = (GetAckTemplate *)buf;

   value.clear();
   value.assign((const char *)temp->value, temp->value_size);

   return (temp->value_size > 0) ? true : false;
}

void Cache::pack_put(const std::string& key, const std::string& value) {
   PutTemplate *format = (PutTemplate *)buf;
   format->job_num = job_num;
   format->job_node = local_rank;
   format->cache_node = coord_rank;
   format->key_size = key.size();
   memcpy(format->key, key.c_str(), key.size());
   format->value_size = value.size();
   memcpy(format->value, value.c_str(), value.size());
   get_timestamp(&(format->timestamp));
}

void Cache::pack_get(const std::string& key) {
   GetTemplate *format = (GetTemplate *)buf;
   format->job_num = job_num;
   format->job_node = local_rank;
   format->key_size = key.size();
   memcpy(format->key, key.c_str(), key.size());
   get_timestamp(&(format->timestamp));
}
