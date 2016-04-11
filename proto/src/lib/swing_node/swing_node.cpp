#include <stdlib.h>
#include "network/network.h"
#include "policy/quorum/swing/swing_quorum.h"
#include "utils/utils.h"
#include "swing_node.h"

SwingNode::SwingNode() {
   // Set the policy for consistency and latency.
   policy = new SwingQuorum(this);

   // Determine where this node is in the system.
   orient();

   // Allocates data structures within this object.
   allocate();

   // Handle all requests sent to this cache node.
   handle_requests();
}

SwingNode::~SwingNode() {
   free(buf);
   delete(policy);
}

void SwingNode::allocate() {
   buf = (uint8_t *)calloc(INITIAL_BUF_SIZE, sizeof(uint8_t));
   ASSERT(buf != NULL, MPI_Abort(MPI_COMM_WORLD, 1));
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

void SwingNode::handle_drop() {
#ifdef DEBUG
   printf("===== DROP =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   fprintf(stderr, "handle_drop not implemented on swing_node!\n");
   ASSERT(1 == 0, MPI_Abort(1, MPI_COMM_WORLD));
}

void SwingNode::handle_drop_ack() {
#ifdef DEBUG
   printf("===== DROP_ACK =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   fprintf(stderr, "handle_drop_ack not implemented on swing_node!\n");
   ASSERT(1 == 0, MPI_Abort(1, MPI_COMM_WORLD));
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
   ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));

#ifdef DEBUG
   printf("SwingNode %d received spawn_job msg successfully!\n", local_rank);
#endif

   // Pull out the job number so we can know which cache nodes to send to.
   SpawnNodesTemplate *format = (SpawnNodesTemplate *)buf;
   uint32_t job_num = format->job_num;

   // There must already have been a CommsGroup entry in the job_to_comms
   // map for this job number (either from cache node creation or from cache
   // node splitting / regrouping in a previous step).
   ASSERT(job_to_comms.count(job_num) != 0, MPI_Abort(MPI_COMM_WORLD, 1));
   MPI_Comm cache_comm = job_to_comms[job_num].cache;

   int comm_size;
   MPI_Comm_remote_size(cache_comm, &comm_size);
   MPI_Request request;

   // Send each cache node a request to spawn a job node.
   for (int i = 0; i < comm_size; ++i) {
#ifdef DEBUG
      printf("SwingNode %d sending spawn cache msg to cache node %d\n",
            local_rank, i);
#endif
      result = send_msg(buf, msg_info.count, MPI_UINT8_T, i, SPAWN_JOB,
                            cache_comm, &request);
      ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));
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
   ASSERT(result == MPI_SUCCESS, MPI_Abort(MPI_COMM_WORLD, 1));

   SpawnNodesTemplate *format = (SpawnNodesTemplate *)buf;
   uint32_t job_num = format->job_num;
   uint16_t node_count = format->count;
   uint16_t mapping_size = format->mapping_size;
   std::string mapping_str(format->mapping, format->mapping + mapping_size);

   // TODO: I don't think we need this anymore
   /*
   std::vector<char> mapping;
   stringlist_to_vector(mapping, mapping_str);
   */

   // Ensure that this job does not have cache nodes already associated with
   // it.
   ASSERT(job_to_comms.count(job_num) == 0, MPI_Abort(MPI_COMM_WORLD, 1));

#ifdef DEBUG
   printf("SwingNode %d msg_info.count: %u\n", local_rank, msg_info.count);
   printf("SwingNode %d mapping_size: %d\n", local_rank, mapping_size);
   printf("SwingNode %d received msg\n", local_rank);
   printf("SwingNode %d job #: %d\n", local_rank, job_num);
   printf("SwingNode %d spawn count: %d\n", local_rank, node_count);
   printf("SwingNode %d mapping: %s\n", local_rank, mapping_str.c_str());
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
   argv[0] = (char *)mapping_str.c_str();
   argv[1] = NULL;

   // All swing nodes spawn the cache nodes.
   MPI_Comm_spawn("./cache_layer", argv, node_count, MPI_INFO_NULL,
         0, temp, &temp, MPI_ERRCODES_IGNORE);

   // Update bookkeeping.
   CommGroup job_comm_group;
   job_comm_group.swing = MPI_COMM_WORLD;
   job_comm_group.cache = temp;
   job_comm_group.job = MPI_COMM_NULL;
   job_to_comms[job_num] = job_comm_group;

#ifdef DEBUG
   printf("SwingNode %d spawned cache_nodes\n", local_rank);
#endif
}

void SwingNode::handle_exit() {
#ifdef DEBUG
   printf("===== EXIT =====\n");
   printf("SwingNode %d\n", local_rank);
   print_msg_info(&msg_info);
#endif

   fprintf(stderr, "handle_exit not implemented on swing_node!\n");
   ASSERT(1 == 0, MPI_Abort(1, MPI_COMM_WORLD));
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

            default:
               printf("===== DEFAULT =====\n");
               fprintf(stderr, "SwingNode %d\n", local_rank);
               fprintf(stderr, "Flag %s was not implemented!\n",
                  msg_tag_handle((MsgTag)msg_info.tag));
               print_msg_info(&msg_info);
               ASSERT(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
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
