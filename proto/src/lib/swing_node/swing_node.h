#ifndef __SWING__NODE__H__
#define __SWING__NODE__H__

#include <iostream>
#include <unordered_map>
#include <mpi.h>
#include <deque>
#include <vector>
#include "policy/policy.h"
#include "policy/quorum.h"
#include "utils/network.h"
#include "utils/node.h"

class SwingNode : public virtual Node {
   public:
      SwingNode();

      ~SwingNode();

   private:
      //MPI_Status status;      // Status structure for checking communications.
      //MPI_Comm parent_comm;   // Intercommunicator between local & parent comm

      uint32_t pair[2];
      uint32_t triple[3];

      //int parent_size;        // Size of parent comm
      //int parent_rank;        // Rank of this node in parent comm
      //int local_size;         // Size of MPI_COMM_WORLD
      //int local_rank;         // Rank of this node in MPI_COMM_WORLD

      // TODO: This is a temporary comm that is used to get things off the
      //       ground, this will be replaced with a service that creates and
      //       tracks unique comms.
      MPI_Comm cache_comm;

      friend class Policy;
      Policy *policy;

      // Struct to hold info about new messages.
      //MsgInfo msg_info;

      // Buffer for holding message data.
      //uint8_t *buf;

      // Queue of messages for the node to handle.
      std::deque<MsgInfo> msg_queue;

      // Map of job_num to cache node communicator.
      //std::unordered_map<uint32_t, MPI_Comm> job_to_comm;

      // Allocates space for dynamic data structures within the object.
      void allocate();

      // Defines the MPI_Datatypes needed for this node to communicate with
      // others.
      //void define_datatypes();

      void handle_team_query();

      void handle_put();

      void handle_put_ack();

      void handle_get();

      void handle_get_ack();

      void handle_forward();

      void handle_forward_ack();

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

      // Prints the current contents of msg_info to stdout.
      //void print_msg_info();
};

#endif

