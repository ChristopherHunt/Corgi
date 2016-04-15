#ifndef __SWING__NODE__H__
#define __SWING__NODE__H__

#include "policy/policy.h"
#include "shared/node.h"

class SwingNode : public virtual Node {
   public:
      SwingNode();

      ~SwingNode();

   private:
      friend class Policy;
      Policy *policy;

      // Flag to tell the node to shutdown.
      bool shutdown;

      // Number of cache nodes spawned by this swing node.
      uint32_t num_cache_nodes;

      // Listing of all unique cache nodes by communicator and list of ranks.
      // Where the index into the list of ranks is the cache node's id, and the
      // value found there is that cache node's coordinator swing node.
      std::unordered_map<MPI_Comm, std::vector<uint32_t> > cache_nodes;

      // Allocates space for dynamic data structures within the object.
      void allocate();

      void handle_team_query();

      void handle_put();

      void handle_put_ack();

      void handle_put_local();

      void handle_put_local_ack();

      void handle_get();

      void handle_get_ack();

      void handle_get_local();

      void handle_get_local_ack();

      void handle_drop();

      void handle_drop_ack();

      void handle_push();

      void handle_push_ack();

      void handle_push_local();

      void handle_push_local_ack();

      void handle_exit();

      void handle_exit_ack();

      void handle_spawn_job();

      void handle_spawn_cache();

      // Handles all message requests from other nodes.
      void handle_requests();

      // Non-blocking polls from the MPI_COMM_WORLD and parent_comm
      // communicators, adding MsgInfo structs to the msg_queue for the main
      // loop to recognize and handle.
      void message_select();

      // Returns true when a message is pending in the msg_queue.
      bool msg_ready();

      // Queries the MPI system to determine where this node is relative to
      // both its peers and its parents.
      void orient();
};

#endif

