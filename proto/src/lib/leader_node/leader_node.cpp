#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include "leader_node.h"

LeaderNode::LeaderNode() {
   // Allocate space for data structures within this object.
   allocate();

   // TODO: Make this job to run specified later during runtime. For now this is
   // just a work around for testing.
   job_exec = "./job_layer";

   // Set tag counter to 0;
   next_job_num = 0;

   // Keep the system up until told otherwise.
   shutdown = false;

   // Update the stdin_delay time.
   get_timestamp(&stdin_delay); 

   // Set initial number of swing nodes to zero.
   num_swing_nodes = 0;

   // Determine where this node is in the system.
   orient();

   // TODO: REMOVE THIS (just for testing)
   create_test_job();

   // Handle all requests sent to this cache node.
   handle_requests();
}

LeaderNode::LeaderNode(const std::string& job_name) {
   // Job to run
   job_exec = job_name;

   // Set tag counter to 0;
   next_job_num = 0;

   // Keep the system up until told otherwise.
   shutdown = false;

   // Update the stdin_delay time.
   get_timestamp(&stdin_delay); 

   // Set initial number of swing nodes to zero.
   num_swing_nodes = 0;

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
   free(buf);
}

void LeaderNode::allocate() {
   buf = (uint8_t *)calloc(INITIAL_BUF_SIZE, sizeof(uint8_t));
   MPI_ASSERT(buf != NULL);
}

// TODO: REMOVE THIS METHOD (It is just for testing functionality).
void LeaderNode::create_test_job() {
   MPI_Comm swing_comm;

   // TODO: This is a simple place holder for swing node spawning,
   //       will want to make this more flexible later.
   spawn_swing_nodes(MPI_COMM_WORLD, &swing_comm, 2);

#ifdef DEBUG
   // TODO: REMOVE
   int size;
   MPI_Comm_remote_size(swing_comm, &size);
   printf("leader after --- swing node count: %d\n", size);
   //
#endif

   // TODO: Make a better way of adding mappings for coordinator nodes.
   //       Use this bandaid to get off the ground for now.
   int job_num = next_job_num++; 
   job_to_comms[job_num].swing = swing_comm; 

   std::vector<uint32_t> temp_vec;
   temp_vec.push_back(0);
   temp_vec.push_back(0);
   temp_vec.push_back(1);
   temp_vec.push_back(1);
   job_to_swing[job_num] = temp_vec;

   // TODO: This is a simple place holder for cache node spawning,
   //       will want to make this more flexible later.
   spawn_cache_nodes(job_num, &swing_comm, 4);

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
   spawn_job_nodes(job_num, job_exec, &swing_comm, 4);
}

void LeaderNode::handle_spawn_job() {
#ifdef DEBUG
   printf("===== SPAWN JOB =====\n");
   printf("LeaderNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   fprintf(stderr, "handle_spawn_job not implemented on leader_node!\n");
   MPI_ASSERT(FAILURE);
}

void LeaderNode::handle_spawn_cache() {
#ifdef DEBUG
   printf("===== SPAWN CACHE =====\n");
   printf("LeaderNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   fprintf(stderr, "handle_spawn_cache not implemented on leader_node!\n");
   MPI_ASSERT(FAILURE);
}

void LeaderNode::handle_exit() {
#ifdef DEBUG
   printf("===== EXIT =====\n");
   printf("LeaderNode %d\n", local_rank);
#endif

   fprintf(stderr, "handle_exit not implemented on leader_node!\n");
   MPI_ASSERT(FAILURE);
}

void LeaderNode::handle_exit_ack() {
#ifdef DEBUG
   printf("===== EXIT_ACK =====\n");
   printf("LeaderNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif
   int result;

   // Recv the message
   result = recv_msg(buf, msg_info.count, MPI_UINT8_T, msg_info.src, EXIT_ACK,
         msg_info.comm, &status);
   ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));

   // TODO: Remove the swing node from the list of swing nodes and incorporate
   // this into logic maybe??

   // decrement num_swing_nodes
   --num_swing_nodes;

   // reply with an exit_ack so it can close
   result = send_msg(buf, msg_info.count, MPI_UINT8_T, msg_info.src, EXIT_ACK,
      msg_info.comm, &request);

   if (num_swing_nodes == 0 && shutdown == true) {
      MPI_Finalize();
   }
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

            case EXIT_ACK:
               handle_exit_ack();
               break;

            default:
               printf("===== DEFAULT =====\n");
               printf("LeaderNode %d\n", local_rank);
               print_msg_info(&msg_info);
               MPI_ASSERT(FAILURE);
               break;
         }
      }
   }
}

void LeaderNode::message_select() {
   int flag;

   for (auto const &entry : job_to_comms) { 
      MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, entry.second.swing, &flag, &status);

      if (flag == 1) {
         msg_info.tag = status.MPI_TAG; 
         msg_info.src = status.MPI_SOURCE;
         msg_info.comm = entry.second.swing;
         MPI_Get_count(&status, MPI_BYTE, &msg_info.count);
         msg_queue.push_back(msg_info);
      }
   }

   uint64_t sys_time;
   get_timestamp(&sys_time);

   // Only check for stdin every STDIN_DELAY amount of time, this should reduce
   // the number of duplicate key-presses registered.
   if (sys_time - stdin_delay > STDIN_DELAY) {
      // Check for user input from stdin, parse any that is found and act
      // accordingly.
      stdin_select();

      // Set the new stdin_delay value.
      stdin_delay = sys_time;
   }

}

void LeaderNode::stdin_select() {
   // Create zeroed timeval struct for select call
   struct timeval tv;
   tv.tv_sec = 0;
   tv.tv_usec = 0; 

   // Create fd_set with STDIN for select call
   fd_set rdfds;
   FD_ZERO(&rdfds);
   FD_SET(STDIN, &rdfds);

   // Check for input on STDIN
   int stdin_ready = select(STDIN + 1, &rdfds, NULL, NULL, &tv);
   MPI_ASSERT(stdin_ready >= 0);

   // If the user input a command, parse it and add it to the message queue if
   // the command is valid.
   if (FD_ISSET(STDIN, &rdfds)) {
      FD_CLR(STDIN, &rdfds);
      // TODO: For now, assume input means kill all processes. Extend this later
      // using getops and some form of syntax.
      handle_stdin();
   }
}

// TODO: CURRENTLY JUST KILLING ALL PROCESSES GRACEFULLY ON STDIN ENTRY FROM
// USER, WILL WANT TO UPDATE THIS LATER.
void LeaderNode::handle_stdin() {
   int result;
   uint16_t comm_node_count;
   int msg_size = sizeof(ExitTemplate);
   MPI_Comm swing_comm = MPI_COMM_NULL;

   // Read whatever was in STDIN to clear buffer
   uint8_t temp[1024];
   result = read(STDIN, temp, 0);
   MPI_ASSERT(result >= 0);

   ExitTemplate *format = (ExitTemplate *)buf;
   format->job_num = KILL_ALL_JOBS;

   // For each communicator of swing nodes
   for (auto const &entry : swing_nodes) { 
      swing_comm = entry.first;
      comm_node_count = entry.second;

      // Send exit request to all swing nodes
      printf("num_swing_nodes: %d\n", comm_node_count);
      for (int i = 0; i < comm_node_count; ++i) {
         printf("Sending swing_comm: %d[%u] exit msg!\n", swing_comm, i);
         result = send_msg(buf, msg_size, MPI_UINT8_T, i, EXIT, swing_comm,
               &request);
         MPI_ASSERT(result == MPI_SUCCESS);
         printf("Msg sent!\n");
      }
   }

   // Tell the system to shutdown once all other nodes have exited.
   shutdown = true;
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

void LeaderNode::spawn_swing_nodes(MPI_Comm parent, MPI_Comm *child,
   uint16_t count) {
   MPI_ASSERT(parent != MPI_COMM_NULL);
   MPI_ASSERT(child != NULL);

   // Duplicate the leader comm so that the original doesn't get cluttered.
   MPI_Comm_dup(parent, child);

   // Spawn the swing node process group.
   MPI_Comm_spawn("./swing_layer", MPI_ARGV_NULL, count, MPI_INFO_NULL, 0,
         *child, child, MPI_ERRCODES_IGNORE);

   // Add the new swing node set to the listing of swing nodes for easy cleanup
   // leater.
   swing_nodes.push_back(std::make_pair(*child, count));

   // Update the number of swing nodes.
   num_swing_nodes += count;
}

void LeaderNode::spawn_cache_nodes(uint32_t job_num, MPI_Comm *comm,
   uint16_t count) {

   MPI_ASSERT(comm != NULL);

#ifdef DEBUG
   printf("LeaderNode sending SPAWN_CACHE of size %d\n", count);
#endif
   int result;
   int comm_size;
   MPI_Comm_remote_size(*comm, &comm_size);

   SpawnNodesTemplate *format = (SpawnNodesTemplate *)buf;
   format->job_num = job_num;
   format->count = count;

   std::vector<uint32_t> vec = job_to_swing[job_num];
   std::string mapping;
   vector_to_stringlist(vec, mapping);
   format->mapping_size = (uint16_t)mapping.size();
   memcpy(format->mapping, mapping.c_str(), mapping.size());

   MPI_ASSERT(mapping.size() <= MAX_MAPPING_SIZE);
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
   // Have all swing nodes collectively spawn the cache nodes.
   for (uint32_t i = 0; i < comm_size; ++i) {
#ifdef DEBUG
      printf("Leader sending spawn cache msg to swing node %d\n", i);
#endif
      result = send_msg(buf, msg_size, MPI_UINT8_T, i, SPAWN_CACHE, *comm,
            &request);
      MPI_ASSERT(result == MPI_SUCCESS);
   }
}

void LeaderNode::spawn_job_nodes(uint32_t job_num, std::string exec_name,
      MPI_Comm *comm, uint16_t count) {

   MPI_ASSERT(comm != NULL);

   int result;

   SpawnNodesTemplate *format = (SpawnNodesTemplate *)buf;
   format->job_num = job_num;
   format->count = count;
   std::vector<uint32_t> vec = job_to_cache[job_num];
   std::string mapping;
   vector_to_stringlist(vec, mapping);
   format->mapping_size = (uint16_t)mapping.size();
   memcpy(format->mapping, mapping.c_str(), mapping.size());
   format->exec_size = (uint8_t)exec_name.size();
   memcpy(format->exec_name, exec_name.c_str(), exec_name.size());
   int msg_size = sizeof(SpawnNodesTemplate);

   MPI_ASSERT(mapping.size() <= MAX_MAPPING_SIZE);
   MPI_ASSERT(exec_name.size() <= MAX_EXEC_NAME_SIZE);

#ifdef DEBUG
   printf("Leader sending spawn job msg to swing node 0\n");
#endif

   // Have the head swing node coordinate all of the cache nodes to spawn the
   // job nodes. This could be streamlined perhaps by distributing the work
   // amongst all of the swing nodes, but at this point the gains in runtime
   // efficiency are miniscule because we aren't starting jobs that often.
   result = send_msg(buf, msg_size, MPI_UINT8_T, 0, SPAWN_JOB, *comm, &request);
   MPI_ASSERT(result == MPI_SUCCESS);
}
