#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include "leader_node.h"

LeaderNode::LeaderNode() {
   // Set tag counter to 0;
   next_job_num = 0;

   // Determine where this node is in the system.
   orient();

   // Allocate space for data structures within this object.
   allocate();

   // TODO: REMOVE THIS (just for testing)
   create_test_job();

   // Handle all requests sent to this cache node.
   handle_requests();
}

LeaderNode::~LeaderNode() {
   //MPI_Type_free(&JOB_EXEC_MSG);
   free(buf);
}

void LeaderNode::allocate() {
   buf = (uint8_t *)calloc(INITIAL_BUF_SIZE, sizeof(uint8_t));
   ASSERT_TRUE(buf != NULL, MPI_Abort(MPI_COMM_WORLD, 1));
}

// TODO: REMOVE THIS METHOD (It is just for testing functionality).
void LeaderNode::create_test_job() {
   MPI_Comm temp;

   // TODO: This is a simple place holder for swing node spawning,
   //       will want to make this more flexible later.
   spawn_swing_nodes(MPI_COMM_WORLD, &temp, 2);

#ifdef DEBUG
   // TODO: REMOVE
   int size;
   MPI_Comm_remote_size(temp, &size);
   printf("leader after --- swing node count: %d\n", size);
   //
#endif

   // TODO: Make a better way of adding mappings for coordinator nodes.
   //       Use this bandaid to get off the ground for now.
   int job_num = next_job_num++; 
   job_to_comm[job_num] = temp; 

   std::vector<uint32_t> temp_vec;
   temp_vec.push_back(0);
   temp_vec.push_back(0);
   temp_vec.push_back(1);
   temp_vec.push_back(1);
   job_to_swing[job_num] = temp_vec;

   // TODO: This is a simple place holder for cache node spawning,
   //       will want to make this more flexible later.
   spawn_cache_nodes(job_num, &temp, 4);

   // TODO: Make a better way of adding mappings for team nodes.
   //       Use this bandaid to get off the ground for now.
   temp_vec.clear();
   temp_vec.push_back(0);
   temp_vec.push_back(1);
   temp_vec.push_back(2);
   temp_vec.push_back(3);
   job_to_cache[job_num] = temp_vec;

   // TODO: This is a simple place holder for job node spawning,
   //       will want to make this more flexible later.
   spawn_job_nodes(job_num, "./job_layer", &temp, 4);
}

void LeaderNode::handle_spawn_job() {
#ifdef DEBUG
   printf("===== SPAWN JOB =====\n");
   printf("LeaderNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif
}

void LeaderNode::handle_spawn_cache() {
#ifdef DEBUG
   printf("===== SPAWN CACHE =====\n");
   printf("LeaderNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif
}

void LeaderNode::handle_exit() {
#ifdef DEBUG
   printf("===== EXIT =====\n");
   printf("LeaderNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif
}

void LeaderNode::handle_requests() {
#ifdef DEBUG
   printf("LeaderNode entering handle_requests!\n");
#endif
   while (true) {
      while (msg_ready() == false) {
         message_select();
      }

      while (msg_ready() == true) {
         msg_info = msg_queue.front();
         msg_queue.pop_front();
#ifdef DEBUG
         printf("msg_queue.size: %u\n", msg_queue.size());
#endif

         switch (msg_info.tag) {
            case SPAWN_JOB:
               handle_spawn_job();
               break;

            case SPAWN_CACHE:
               handle_spawn_cache();
               break;

            case EXIT:
               handle_exit();
               break;

            default:
               printf("===== DEFAULT =====\n");
               printf("LeaderNode %d\n", local_rank);
               print_msg_info(&msg_info);
               ASSERT_TRUE(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
               break;
         }
      }
   }
}

void LeaderNode::message_select() {
   int flag;

   for (auto const &entry : job_to_comm) { 
      MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, entry.second, &flag, &status);

      if (flag == 1) {
         msg_info.tag = status.MPI_TAG; 
         msg_info.src = status.MPI_SOURCE;
         msg_info.comm = entry.second;
         MPI_Get_count(&status, MPI_BYTE, &msg_info.count);
         msg_queue.push_back(msg_info);
      }
   }
}

bool LeaderNode::msg_ready() {
   return msg_queue.size() > 0 ? true : false;
}

void LeaderNode::orient() {
   // Get data on local comm
   MPI_Comm_size(MPI_COMM_WORLD, &local_size);
   MPI_Comm_rank(MPI_COMM_WORLD, &local_rank);

   if (local_size != 1) {
      printf("Currently do not support multiple leader nodes, exiting!\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
   }
}

void LeaderNode::spawn_swing_nodes(MPI_Comm parent, MPI_Comm *child, uint16_t count) {
   // TODO: Make it so we can get unique comm handles prior to placing them in
   //       the swing comm queue. For now just hardcode a name to make things
   //       easier for testing.
   MPI_Comm_dup(parent, child);

   MPI_Comm_spawn("./swing_layer", MPI_ARGV_NULL, count, MPI_INFO_NULL, 0,
         *child, child, MPI_ERRCODES_IGNORE);
}

void LeaderNode::spawn_cache_nodes(uint32_t job_num, MPI_Comm *comm, uint16_t count) {
#ifdef DEBUG
   printf("LeaderNode sending SPAWN_CACHE of size %d\n", count);
#endif
   int comm_size;
   MPI_Comm_remote_size(*comm, &comm_size);

   SpawnNodesTemplate *format = (SpawnNodesTemplate *)buf;
   format->job_num = job_num;
   format->count = count;

   std::vector<uint32_t> vec = job_to_swing[job_num];
   std::string result;
   vector_to_stringlist(vec, result);
   format->mapping_size = (uint16_t)result.size();
   memcpy(format->mapping, result.c_str(), result.size());

   ASSERT_TRUE(result.size() <= MAX_MAPPING_SIZE, MPI_Abort(MPI_COMM_WORLD, 1));
   int msg_size = sizeof(SpawnNodesTemplate);
#ifdef DEBUG
   printf("job_num: %d\n", job_num);
   printf("count: %d\n", count);
   printf("spawn_cache_msg_size: %d\n", msg_size);
   printf("job_num: %d\ncount: %d\nmapping_size: %d\nmapping: %s\n",
         format->job_num, format->count, format->mapping_size, format->mapping);
#endif

   // TODO: Look into MPI_Comm_Idup and perhaps MPI_Bcast for sending out this
   //       spawn request to all nodes efficiently and having them all handle it
   //       efficiently.
   MPI_Request request; 
   // Have all swing nodes collectively spawn the cache nodes.
   for (uint32_t i = 0; i < comm_size; ++i) {
#ifdef DEBUG
      printf("Leader sending spawn cache msg to swing node %d\n", i);
#endif
      send_msg(buf, msg_size, MPI_UINT8_T, i, SPAWN_CACHE, *comm, &request);
   }
}

void LeaderNode::spawn_job_nodes(uint32_t job_num, std::string exec_name,
      MPI_Comm *comm, uint16_t count) {

   SpawnNodesTemplate *format = (SpawnNodesTemplate *)buf;
   format->job_num = job_num;
   format->count = count;
   std::vector<uint32_t> vec = job_to_cache[job_num];
   std::string result;
   vector_to_stringlist(vec, result);
   format->mapping_size = (uint16_t)result.size();
   memcpy(format->mapping, result.c_str(), result.size());
   format->exec_size = (uint8_t)exec_name.size();
   memcpy(format->exec_name, exec_name.c_str(), exec_name.size());
   int msg_size = sizeof(SpawnNodesTemplate);

   ASSERT_TRUE(result.size() <= MAX_MAPPING_SIZE, MPI_Abort(MPI_COMM_WORLD, 1));
   ASSERT_TRUE(exec_name.size() <= MAX_EXEC_NAME_SIZE, MPI_Abort(MPI_COMM_WORLD, 1));

#ifdef DEBUG
   printf("Leader sending spawn job msg to swing node 0\n");
#endif

   // Have the head swing node coordinate all of the cache nodes to spawn the
   // job nodes. This could be streamlined perhaps by distributing the work
   // amongst all of the swing nodes, but at this point the gains in runtime
   // efficiency are miniscule because we aren't starting jobs that often.
   MPI_Request request;
   send_msg(buf, msg_size, MPI_UINT8_T, 0, SPAWN_JOB, *comm, &request);
}
