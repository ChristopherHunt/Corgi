#include <stdlib.h>
#include "policy/quorum/swing/swing_quorum.h"
#include "swing_node.h"

SwingNode::SwingNode() {
   // Allocates data structures within this object.
   allocate();

   // Determine where this node is in the system.
   orient();

   // Keep the node up unless told otherwise.
   shutdown = false;

   // Set number of initial cache nodes to zero.
   num_cache_nodes = 0;

   // Handle all requests sent to this cache node.
   handle_requests();
}

SwingNode::~SwingNode() {
   free(buf);
   delete(policy);
   printf("SwingNode %d is exiting!\n", local_rank);
}

void SwingNode::allocate() {
   buf = (uint8_t *)calloc(INITIAL_BUF_SIZE, sizeof(uint8_t));
   MPI_ASSERT(buf != NULL);

   // Set the policy for consistency and latency.
   policy = new SwingQuorum(this);
   MPI_ASSERT(policy != NULL);
}

void SwingNode::handle_put() {
#ifdef DEBUG
   printf("===== PUT =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_put();
}

void SwingNode::handle_put_ack() {
#ifdef DEBUG
   printf("===== PUT ACK =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_put_ack();
}

void SwingNode::handle_put_local() {
#ifdef DEBUG
   printf("===== PUT_LOCAL =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_put_local();
}

void SwingNode::handle_put_local_ack() {
#ifdef DEBUG
   printf("===== PUT LOCAL ACK =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_put_local_ack();
}

void SwingNode::handle_get() {
#ifdef DEBUG
   printf("===== GET =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_get();
}

void SwingNode::handle_get_ack() {
#ifdef DEBUG
   printf("===== GET ACK =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_get_ack();
}

void SwingNode::handle_get_local() {
#ifdef DEBUG
   printf("===== GET LOCAL =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_get_local();
}

void SwingNode::handle_get_local_ack() {
#ifdef DEBUG
   printf("===== GET LOCAL ACK =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_get_local_ack();
}

void SwingNode::handle_push() {
#ifdef DEBUG
   printf("===== PUSH =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_push();
}

void SwingNode::handle_push_ack() {
#ifdef DEBUG
   printf("===== PUSH_ACK =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_push_ack();
}

void SwingNode::handle_push_local() {
#ifdef DEBUG
   printf("===== PUSH_LOCAL =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_push_local();
}

void SwingNode::handle_push_local_ack() {
#ifdef DEBUG
   printf("===== PUSH_LOCAL_ACK =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   policy->handle_push_local_ack();
}

void SwingNode::handle_drop() {
#ifdef DEBUG
   printf("===== DROP =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   fprintf(stderr, "handle_drop not implemented on swing_node!\n");
   MPI_ASSERT(FAILURE);
}

void SwingNode::handle_drop_ack() {
#ifdef DEBUG
   printf("===== DROP_ACK =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   fprintf(stderr, "handle_drop_ack not implemented on swing_node!\n");
   MPI_ASSERT(FAILURE);
}

void SwingNode::handle_team_query() {
#ifdef DEBUG
   printf("===== TEAM QUERY =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif
}

void SwingNode::handle_spawn_job() {
#ifdef DEBUG
   printf("===== SPAWN JOB =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif
   int result;

   // Receive the job spawn request from the leader.
   result = recv_msg(buf, msg_info.count, MPI_UINT8_T, msg_info.src, SPAWN_JOB,
         msg_info.comm, &status);
   MPI_ASSERT(result == MPI_SUCCESS);

#ifdef DEBUG
   printf("SwingNode %d received spawn_job msg successfully!\n", local_rank);
#endif

   // Pull out the job number so we can know which cache nodes to send to.
   SpawnNodesTemplate *format = (SpawnNodesTemplate *)buf;
   uint32_t job_num = format->job_num;

   // There must already have been a CommsGroup entry in the job_to_comms
   // map for this job number (either from cache node creation or from cache
   // node splitting / regrouping in a previous step).
   MPI_ASSERT(job_to_comms.count(job_num) != 0);
   MPI_Comm cache_comm = job_to_comms[job_num].cache;

   int comm_size;
   MPI_Comm_remote_size(cache_comm, &comm_size);

   // Send each cache node a request to spawn a job node.
   for (int i = 0; i < comm_size; ++i) {
#ifdef DEBUG
      printf("SwingNode %d sending spawn cache msg to cache node %d\n",
            local_rank, i);
#endif
      result = send_msg(buf, msg_info.count, MPI_UINT8_T, i, SPAWN_JOB,
            cache_comm, &request);
      MPI_ASSERT(result == MPI_SUCCESS);
   }
}

void SwingNode::handle_spawn_cache() {
#ifdef DEBUG
   printf("===== SPAWN CACHE =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   int result;

   result = recv_msg(buf, msg_info.count, MPI_UINT8_T, msg_info.src,
         SPAWN_CACHE, msg_info.comm, &status);
   MPI_ASSERT(result == MPI_SUCCESS);

   SpawnNodesTemp spawn_nodes;
   SpawnNodesTemplate *format = (SpawnNodesTemplate *)buf;
   spawn_nodes.unpack(format);
   /*
   uint32_t job_num = format->job_num;
   uint16_t node_count = format->count;
   uint16_t mapping_size = format->mapping_size;
   std::string mapping_str(format->mapping, format->mapping + mapping_size);
   */

   // Ensure that this job does not have cache nodes already associated with
   // it.
   MPI_ASSERT(job_to_comms.count(spawn_nodes.job_num) == 0);

#ifdef DEBUG
   printf("SwingNode %d msg_info.count: %u\n", local_rank, msg_info.count);
   printf("SwingNode %d mapping_size: %d\n", local_rank, spawn_nodes.mapping_size);
   printf("SwingNode %d received msg\n", local_rank);
   printf("SwingNode %d job #: %d\n", local_rank, spawn_nodes.job_num);
   printf("SwingNode %d spawn count: %d\n", local_rank, spawn_nodes.count);
   printf("SwingNode %d mapping: %s\n", local_rank, spawn_nodes.mapping.c_str());
   printf("SwingNode %d mapping as a vector:\n", local_rank);
   /*
      for (std::vector<char>::iterator it = mapping.begin();
      it != mapping.end(); ++it) {
      std::cout << *it << std::endl;
      }
      std::cout << std::endl;
      */
#endif

   // Create a new communicator to spawn the cache nodes off of, since we don't
   // want to extend the same MPI_COMM_WORLD again and again.
   MPI_Comm temp;
   MPI_Comm_dup(MPI_COMM_WORLD, &temp);

   // Create an array that maps each cache node to its corresponding
   // coordinator swing node, and pass that to the nodes upon spawning.
   char *argv[2];
   argv[0] = (char *)spawn_nodes.mapping.c_str();
   argv[1] = NULL;

   // All swing nodes spawn the cache nodes.
   MPI_Comm_spawn("./cache_layer", argv, spawn_nodes.count, MPI_INFO_NULL,
         0, temp, &temp, MPI_ERRCODES_IGNORE);

   // Store the mapping of swing nodes to cache nodes.
   std::vector<uint32_t> mapping;
   stringlist_to_vector(mapping, spawn_nodes.mapping);
   cache_nodes[temp] = mapping;
   num_cache_nodes += spawn_nodes.count;

   // Update bookkeeping.
   CommGroup job_comm_group;
   job_comm_group.swing = MPI_COMM_WORLD;
   job_comm_group.cache = temp;
   job_comm_group.job = MPI_COMM_NULL;
   job_to_comms[spawn_nodes.job_num] = job_comm_group;

#ifdef DEBUG
   printf("SwingNode %d spawned cache_nodes\n", local_rank);
#endif
}

// TODO: CURRENTLY JUST KILLING ALL PROCESSES GRACEFULLY, WILL WANT TO MAKE THIS
// MORE FLEXIBLE LATER BY LOOKING AT THE JOB_NUM ITSELF.
void SwingNode::handle_exit() {
#ifdef DEBUG
   printf("===== EXIT =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif
   int result;
   int msg_size = sizeof(ExitTemplate);
   int comm_node_count;
   std::vector<uint32_t> node_mapping;
   MPI_Comm cache_comm;

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
         for (auto const &entry : cache_nodes) { 
            cache_comm = entry.first;
            node_mapping = entry.second;
            comm_node_count = node_mapping.size();

            // Send exit request to cache nodes
            for (int i = 0; i < comm_node_count; ++i) {
               // Only send to the cache nodes you coordinate
               if (node_mapping[i] == local_rank) {
                  printf("SwingNode %d sending cache_comm: %d[%u] exit msg!\n",
                     local_rank, cache_comm, i);
                  result = send_msg(buf, msg_size, MPI_UINT8_T, i, EXIT,
                     cache_comm, &request);
                  MPI_ASSERT(result == MPI_SUCCESS);
                  printf("Msg sent!\n");
               }
            }
         }

         // Mark the node to shutdown.
         shutdown = true;
      }
      // Try and kill the specified nodes.
      else {
         fprintf(stderr, "SwingNode handling exit msgs to kill specific nodes unimplemented!\n");
         MPI_ASSERT(FAILURE);
      }
   }
   // If this is coming from a child then it means the child is exiting on its
   // own free will.
   else {
      fprintf(stderr, "SwingNode handling exit msgs from child nodes unimplemented!\n");
      MPI_ASSERT(FAILURE);
      // Remove that node from the mapping of nodes and free all resources tied
      // to it.
      // Once all nodes in a job are removed, free up the resources associated
      // with that job as a whole
   }
}

void SwingNode::handle_exit_ack() {
#ifdef DEBUG
   printf("===== EXIT_ACK =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif
   int result;

   // Recv the message
   result = recv_msg(buf, msg_info.count, MPI_UINT8_T, msg_info.src, EXIT_ACK,
         msg_info.comm, &status);
   MPI_ASSERT(result == MPI_SUCCESS);

   // TODO: Remove the cache node from the list of cache nodes and incorporate
   // this into logic maybe??

   // decrement num_swing_nodes
   --num_cache_nodes;

   // reply with an exit_ack so it can close
   result = send_msg(buf, msg_info.count, MPI_UINT8_T, msg_info.src, EXIT_ACK,
      msg_info.comm, &request);

   if (num_cache_nodes == 0 && shutdown == true) {
      MPI_Finalize();
   }
}

void SwingNode::handle_requests() {
#ifdef DEBUG
   printf("SwingNode %d entering handle_requests!\n", local_rank);
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

            case DROP:
               handle_drop();
               break;

            case DROP_ACK:
               handle_drop_ack();
               break;

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
               fprintf(stderr, "SwingNode %d\n", local_rank);
               fprintf(stderr, "Flag %s was not implemented!\n",
                     msg_tag_handle((MsgTag)msg_info.tag));
               print_msg_info(&msg_info);
               MPI_ASSERT(FAILURE);
               break;
         }
      }
   }
}

void SwingNode::message_select() {
   int flag;
   MPI_Comm comm;

   // Check to see if the leader node is trying to talk to you.
   MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, parent_comm, &flag, &status);

   if (flag == 1) {
      msg_info.tag = status.MPI_TAG; 
      msg_info.src= status.MPI_SOURCE;
      msg_info.comm = parent_comm;
      MPI_Get_count(&status, MPI_BYTE, &msg_info.count);
      msg_queue.push_back(msg_info);
   }

   // Check to see if any swing nodes are trying to talk to you.
   MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

   if (flag == 1) {
      msg_info.tag = status.MPI_TAG; 
      msg_info.src = status.MPI_SOURCE;
      msg_info.comm = MPI_COMM_WORLD;
      MPI_Get_count(&status, MPI_BYTE, &msg_info.count);
      msg_queue.push_back(msg_info);
   }

   // Check to see if any cache nodes are trying to talk to you.
   for (auto const &entry : job_to_comms) {
      comm = entry.second.cache;
      MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &flag, &status);

      if (flag == 1) {
         msg_info.tag = status.MPI_TAG; 
         msg_info.src = status.MPI_SOURCE;
         msg_info.comm = comm;
         MPI_Get_count(&status, MPI_BYTE, &msg_info.count);
         msg_queue.push_back(msg_info);
      }
   }
}

bool SwingNode::msg_ready() {
   return msg_queue.size() > 0 ? true : false;
}

void SwingNode::orient() {
   // Get data on local comm
   MPI_Comm_size(MPI_COMM_WORLD, &local_size);
   MPI_Comm_rank(MPI_COMM_WORLD, &local_rank);

#ifdef DEBUG
   printf("++++++> SwingNode %d here!\n", local_rank);
#endif

   // Get parent comm
   MPI_Comm_get_parent(&parent_comm);

   if (parent_comm == MPI_COMM_NULL) {
      printf("SwingNode %d could not access parent comm.\n", local_rank);
      MPI_Abort(MPI_COMM_WORLD, 1);
   }

   // Get data on parent comm
   MPI_Comm_remote_size(parent_comm, &parent_size);
   MPI_Comm_rank(parent_comm, &parent_rank);
}
