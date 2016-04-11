#ifndef __CACHE__NODE__H__
#define __CACHE__NODE__H__

#include <iostream>
#include <unordered_map>
#include <mpi.h>
#include <deque>
#include <vector>
#include "policy/policy.h"
#include "network/network.h"
#include "shared/node.h"

class CacheNode : public virtual Node {
   public:
      // Constructs a Cache node, referencing a swing node in its parent
      // communicator who's id is held within the mapping vector.
      CacheNode(std::vector<uint32_t>& mapping);

      ~CacheNode();

   private:
      friend class Policy;
      Policy *policy;

      // Allocates space for dynamic data structures within the object.
      void allocate();

      // Handles incoming messages requesting to "put" a key/value pair into the
      // cache.
      void handle_put();

      // Handle acknowledgements to a "put" message which may have propogated
      // through the cache.
      // TODO: NOT SURE IF THIS IS NEEDED IN THE CACHE LAYER ANYMORE
      void handle_put_ack();

      // Handles incoming messages request to "get" a value from the cache
      // corresponding to a specific key.
      void handle_get();

      // Handle acknowledgements to a "get" message which may have propogated
      // through the cache.
      void handle_get_ack();

      void handle_put_local();

      void handle_put_local_ack();

      void handle_get_local();

      void handle_get_local_ack();

      void handle_push();

      void handle_push_ack();

      void handle_drop();

      void handle_drop_ack();

      void handle_spawn_job();

      void handle_exit();

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

      // Creates TCP socket for Job nodes to talk with this cache node. Also
      // communicates this information with its coordinator swing node.
      void setup_socket();
};

#endif

