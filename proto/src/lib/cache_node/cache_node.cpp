#include <stdio.h>
#include <stdlib.h>
#include "policy/quorum/cache/cache_quorum.h"
#include "cache_node/cache_node.h"

// TODO: NEED TO ADD FUNCTIONALITY TO ADD JOBS WITHOUT ADDING CACHE NODES.

CacheNode::CacheNode(std::vector<uint32_t>& mapping) {
   // Allocates data structures within this object.
   allocate();

   // Keep the node up unless told otherwise.
   shutdown = false;

   // Set number of initial job nodes to zero.
   num_job_nodes = 0;

   // Determine where this node is in the system.
   orient();

   // Get coordinator swing node's rank
   coord_rank = mapping[local_rank];

#ifdef DEBUG
   printf("CacheNode %d's coord swing node is %d\n", local_rank, coord_rank);
#endif

   // Handle all requests sent to this cache node.
   handle_requests();
}

CacheNode::~CacheNode() {
   free(buf);
   delete(policy);
}

void CacheNode::allocate() {
   buf = (uint8_t *)calloc(INITIAL_BUF_SIZE, sizeof(uint8_t));
   MPI_ASSERT(buf != NULL);

   // Set the policy for consistency and latency.
   policy = new CacheQuorum(this);
   MPI_ASSERT(policy != NULL);
}

void CacheNode::handle_put() {
#ifdef DEBUG
   printf("===== PUT =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_put();
}

void CacheNode::handle_put_ack() {
#ifdef DEBUG
   printf("===== PUT_ACK =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_put_ack();
}

void CacheNode::handle_put_local() {
#ifdef DEBUG
   printf("===== PUT_LOCAL =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_put_local();
}

void CacheNode::handle_put_local_ack() {
#ifdef DEBUG
   printf("===== PUT_LOCAL_ACK =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_put_local_ack();
}

void CacheNode::handle_get() {
#ifdef DEBUG
   printf("===== GET =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_get();
}

void CacheNode::handle_get_ack() {
#ifdef DEBUG
   printf("===== GET_ACK =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_get_ack();
}

void CacheNode::handle_get_local() {
#ifdef DEBUG
   printf("===== GET_LOCAL =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_get_local();
}

void CacheNode::handle_get_local_ack() {
#ifdef DEBUG
   printf("===== GET_LOCAL_ACK =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_get_local_ack();
}

void CacheNode::handle_push() {
#ifdef DEBUG
   printf("===== PUSH =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_push();
}

void CacheNode::handle_push_ack() {
#ifdef DEBUG
   printf("===== PUSH_ACK =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_push_ack();
}

void CacheNode::handle_push_local() {
#ifdef DEBUG
   printf("===== PUSH_LOCAL =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_push_local();
}

void CacheNode::handle_push_local_ack() {
#ifdef DEBUG
   printf("===== PUSH_LOCAL_ACK =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_push_local_ack();
}

void CacheNode::handle_pull() {
#ifdef DEBUG
   printf("===== PULL =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_pull();
}

void CacheNode::handle_pull_ack() {
#ifdef DEBUG
   printf("===== PULL_ACK =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_pull_ack();
}

void CacheNode::handle_pull_local() {
#ifdef DEBUG
   printf("===== PULL_LOCAL =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_pull_local();
}

void CacheNode::handle_pull_local_ack() {
#ifdef DEBUG
   printf("===== PULL_LOCAL_ACK =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_pull_local_ack();
}

void CacheNode::handle_drop() {
#ifdef DEBUG
   printf("===== DROP =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_drop();
}

void CacheNode::handle_drop_ack() {
#ifdef DEBUG
   printf("===== DROP_ACK =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_drop_ack();
}

void CacheNode::handle_drop_local() {
#ifdef DEBUG
   printf("===== DROP_LOCAL =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_drop_local();
}

void CacheNode::handle_drop_local_ack() {
#ifdef DEBUG
   printf("===== DROP_LOCAL_ACK =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_drop_local_ack();
}

void CacheNode::handle_exit() {
#ifdef DEBUG
   printf("===== EXIT =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif
   int result;
   //int msg_size = sizeof(ExitTemplate);
   //int comm_node_count;
   std::vector<uint32_t> node_mapping;
   MPI_Comm job_comm;

   // Recv the message
   result = recv_msg(buf, msg_info.count, MPI_UINT8_T, msg_info.src, EXIT,
         msg_info.comm, &status);
   MPI_ASSERT(result == MPI_SUCCESS);

   ExitTemplate *format = (ExitTemplate *)buf;

   // If this exit request is coming from above then it means to shut things
   // down practively.
   if (msg_info.comm == parent_comm) {
      // If the parent is telling us to kill everything, then kill everything
      if (format->job_num == KILL_ALL_JOBS) {
         // For each communicator of swing nodes
         for (auto const &entry : job_nodes) { 
            job_comm = entry.first;
            // TODO: For now just calling MPI_Abort, but realistically want to
            // update the cache_api constructor so it spawns a thread which
            // listens for an exit command from the cache layer. Once that
            // exists we can exit cleanly.
            MPI_Abort(job_comm, 1);
            /*
            node_mapping = entry.second;
            comm_node_count = node_mapping.size();

            // Send exit request to cache nodes
            for (int i = 0; i < comm_node_count; ++i) {
               // Only send to the cache nodes you coordinate
               if (node_mapping[i] == local_rank) {
                  printf("CacheNode %d sending job_comm: %d[%u] exit msg!\n",
                     local_rank, job_comm, i);
                  result = send_msg(buf, msg_size, MPI_UINT8_T, i, EXIT,
                     job_comm, &request);
                  ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));
                  printf("Msg sent!\n");
               }
            }
            */
         }

         // Mark the node to shutdown.
         shutdown = true;
      }
      // Try and kill the specified nodes.
      else {
         fprintf(stderr, "CacheNode handling exit msgs to kill specific nodes unimplemented!\n");
         MPI_ASSERT(FAILURE);
      }
   }
   // If this is coming from a child then it means the child is exiting on its
   // own free will.
   else {
      fprintf(stderr, "CacheNode handling exit msgs from child nodes unimplemented!\n");
      MPI_ASSERT(FAILURE);
      // Remove that node from the mapping of nodes and free all resources tied
      // to it.
      // Once all nodes in a job are removed, free up the resources associated
      // with that job as a whole
   }
}

void CacheNode::handle_exit_ack() {
#ifdef DEBUG
   printf("===== EXIT_ACK =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   fprintf(stderr, "handle_exit_ack not implemented on cache_node!\n");
   MPI_ASSERT(FAILURE);
   /*
   int result;

   // Recv the message
   result = recv_msg(buf, msg_info.count, MPI_UINT8_T, msg_info.src, EXIT_ACK,
         msg_info.comm, &status);
   ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));

   // TODO: Remove the cache node from the list of cache nodes and incorporate
   // this into logic maybe??

   // decrement num_swing_nodes
   --num_job_nodes;

   // reply with an exit_ack so it can close
   result = send_msg(buf, msg_info.count, MPI_UINT8_T, msg_info.src, EXIT_ACK,
      msg_info.comm, &request);

   if (num_cache_nodes == 0 && shutdown == true) {
      MPI_Finalize();
   }
   */
}

void CacheNode::handle_requests() {
#ifdef DEBUG
   printf("CacheNode %d entering handle_requests!\n", local_rank);
#endif
   while (true) {
      while (msg_ready() == false) {
         message_select();
      }

      while (msg_ready() == true) {
         msg_info = msg_queue.front();
         msg_queue.pop_front();

         switch (msg_info.tag) {
            case PUT:
               handle_put();
               break;

            case PUT_ACK:
               handle_put_ack();
               break;

            case PUT_LOCAL:
               handle_put_local();
               break;

            case PUT_LOCAL_ACK:
               handle_put_local_ack();
               break;

            case GET:
               handle_get();
               break;

            case GET_ACK:
               handle_get_ack();
               break;

            case GET_LOCAL:
               handle_get_local();
               break;

            case GET_LOCAL_ACK:
               handle_get_local_ack();
               break;

            case PUSH:
               handle_push();
               break;

            case PUSH_ACK:
               handle_push_ack();
               break;

            case PUSH_LOCAL:
               handle_push_local();
               break;

            case PUSH_LOCAL_ACK:
               handle_push_local_ack();
               break;

            case PULL:
               handle_pull();
               break;

            case PULL_ACK:
               handle_pull_ack();
               break;

            case PULL_LOCAL:
               handle_pull_local();
               break;

            case PULL_LOCAL_ACK:
               handle_pull_local_ack();
               break;

            case DROP:
               handle_drop();
               break;

            case DROP_ACK:
               handle_drop_ack();
               break;
               
            case DROP_LOCAL:
               handle_drop_local();
               break;

            case DROP_LOCAL_ACK:
               handle_drop_local_ack();
               break;

            case SPAWN_JOB:
               handle_spawn_job();
               break;

            case EXIT:
               handle_exit();
               break;

            default:
               printf("===== DEFAULT =====\n");
               fprintf(stderr, "CacheNode %d\n", local_rank);
               fprintf(stderr, "Flag %s was not implemented!\n",
                  msg_tag_handle((MsgTag)msg_info.tag));
               print_msg_info(&msg_info);
               MPI_ASSERT(FAILURE);
               break;
         }
      }
   }
}

void CacheNode::handle_spawn_job() {
#ifdef DEBUG
   printf("===== SPAWN JOB =====\n");
   printf("CacheNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   int result;

   result = recv_msg(buf, msg_info.count, MPI_UINT8_T, msg_info.src, SPAWN_JOB,
         msg_info.comm, &status);
   MPI_ASSERT(result == MPI_SUCCESS);

   // Parse the template to determine how to spawn the job
   SpawnNodesTemp spawn_nodes;
   SpawnNodesTemplate *format = (SpawnNodesTemplate *)buf;
   spawn_nodes.unpack(format);

#ifdef DEBUG
   printf("CacheNode %d msg_info.count: %u\n", local_rank, msg_info.count);
   printf("CacheNode %d mapping_size: %d\n", local_rank, spawn_nodes.mapping_size);
   printf("CacheNode %d received msg\n", local_rank);
   printf("CacheNode %d job #: %d\n", local_rank, spawn_nodes.job_num);
   printf("CacheNode %d spawn count: %d\n", local_rank, spawn_nodes.count);
   printf("CacheNode %d mapping: %s\n", local_rank, spawn_nodes.mapping.c_str());
   printf("CacheNode %d mapping as a vector:\n", local_rank);
   for (std::vector<char>::iterator it = spawn_nodes.mapping.begin();
         it != spawn_nodes.mapping.end(); ++it) {
      std::cout << *it << std::endl;
   }
   std::cout << std::endl;
   printf("CacheNode %d exec_size: %d\n", local_rank, spawn_nodes.exec_size);
   printf("CacheNode %d exec_name: %s\n", local_rank, spawn_nodes.exec_name.c_str());
#endif


   std::stringstream ss;
   ss << spawn_nodes.job_num;
   char *job_num_str = (char *)ss.str().c_str();

   // Create an array that maps each job node to its corresponding cache node.
   char *argv[3];
   argv[0] = job_num_str;
   argv[1] = (char *)(spawn_nodes.mapping.c_str());
   argv[2] = NULL;

   // Duplicate MPI_COMM_WORLD so that we don't extend it again and again with
   // newly spawned jobs.
   MPI_Comm comm;
   MPI_Comm_dup(MPI_COMM_WORLD, &comm);

   // Spawn the job nodes with the appropriate argv mapping.
   MPI_Comm_spawn(spawn_nodes.exec_name.c_str(), argv, spawn_nodes.count,
      MPI_INFO_NULL, 0, comm, &comm, MPI_ERRCODES_IGNORE);

   // Store the mapping of cache nodes to job nodes.
   std::vector<uint32_t> mapping;
   stringlist_to_vector(mapping, spawn_nodes.mapping);
   job_nodes[comm] = mapping;
   num_job_nodes += spawn_nodes.count;

   // Update bookkeeping.
   CommGroup job_comm_group;
   job_comm_group.swing = msg_info.comm;
   job_comm_group.cache = MPI_COMM_WORLD;
   job_comm_group.job = comm;
   job_to_comms[spawn_nodes.job_num] = job_comm_group;

#ifdef DEBUG
   printf("CacheNode %d finished spawning Job %d\n", local_rank, spawn_nodes.job_num);
#endif
}

void CacheNode::message_select() {
   int flag;

   // Check to see if any cache nodes are attempting to talk to you.
   MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

   if (flag == 1) {
      msg_info.tag = status.MPI_TAG; 
      msg_info.src = status.MPI_SOURCE;
      msg_info.comm = MPI_COMM_WORLD;
      MPI_Get_count(&status, MPI_BYTE, &msg_info.count);
      msg_queue.push_back(msg_info);
   }

   // Check to see if any swing nodes are attempting to talk to you.
   MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, parent_comm, &flag, &status);

   if (flag == 1) {
      msg_info.tag = status.MPI_TAG; 
      msg_info.src= status.MPI_SOURCE;
      msg_info.comm = parent_comm;
      MPI_Get_count(&status, MPI_BYTE, &msg_info.count);
      msg_queue.push_back(msg_info);
   }

   // Check to see if any job nodes are attempting to talk to you.
   for (auto const &entry : job_to_comms) {
      MPI_ASSERT(entry.second.job != MPI_COMM_NULL);

      MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, entry.second.job, &flag,
            &status);

      if (flag == 1) {
         msg_info.tag = status.MPI_TAG; 
         msg_info.src = status.MPI_SOURCE;
         msg_info.comm = entry.second.job;
         MPI_Get_count(&status, MPI_BYTE, &msg_info.count);
         msg_queue.push_back(msg_info);
      }
   }
}

bool CacheNode::msg_ready() {
   return msg_queue.size() > 0 ? true : false;
}

void CacheNode::orient() {
   // Get data on local comm
   MPI_Comm_size(MPI_COMM_WORLD, &local_size);
   MPI_Comm_rank(MPI_COMM_WORLD, &local_rank);

   // Get parent comm
   MPI_Comm_get_parent(&parent_comm);

   if (parent_comm == MPI_COMM_NULL) {
      printf("CacheNode %d could not access parent comm.\n", local_rank);
      MPI_Abort(MPI_COMM_WORLD, 1);
   }

   // Get data on parent comm
   MPI_Comm_remote_size(parent_comm, &parent_size);
   MPI_Comm_rank(parent_comm, &parent_rank);

#ifdef DEBUG
   printf("CacheNode %d see's parent's comm size: %d\n", local_rank, parent_size); 
#endif
}
