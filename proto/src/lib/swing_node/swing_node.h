#ifndef __SWING__NODE__H__
#define __SWING__NODE__H__

#include <iostream>
#include <unordered_map>
#include <mpi.h>
#include <deque>
#include <vector>
#include "policy/policy.h"
#include "network/network.h"
#include "shared/node.h"

class SwingNode : public virtual Node {
   public:
      SwingNode();

      ~SwingNode();

   private:
      friend class Policy;
      Policy *policy;

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

      void handle_exit();

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

