#ifndef __LEADER__NODE__H__
#define __LEADER__NODE__H__

#include <stdint.h>
#include "shared/node.h"

class LeaderNode : public Node {
   public:
      LeaderNode();

      LeaderNode(const std::string& job_exec);

      ~LeaderNode();

   private:
      // Name of job exec to be run. This is for testing.
      std::string job_exec;

      // Flag to tell the system to shutdown.
      bool shutdown;

      // Number of swing nodes spawned by leader.
      uint32_t num_swing_nodes;

      // Timestamp used to handle/throttle stdin.
      uint64_t stdin_delay;

      // A counter for assigning new job tags.
      uint32_t next_job_num;

      // Map of job tag to map of cache node -> coord swing node.
      std::unordered_map<uint32_t, std::vector<uint32_t> > job_to_swing;

      // Map of job tag to team node pairings (job node -> cache_node).
      std::unordered_map<uint32_t, std::vector<uint32_t> > job_to_cache;

      // Listing all of unique swing nodes by communicator and number of nodes.
      std::vector<std::pair<MPI_Comm, uint16_t> > swing_nodes;

      // Allocates space for dynamic data structures within the object.
      void allocate();

      // Method which creates a hard-coded job for cache testing.
      void create_test_job();

      void handle_spawn_job();

      void handle_spawn_cache();

      void handle_exit();

      void handle_stdin();

      void handle_exit_ack();

      // Handles all message requests from other nodes.
      void handle_requests();

      // Creates a communicator and spawns a set of swing nodes.
      void spawn_swing_nodes(MPI_Comm parent, MPI_Comm *child, uint16_t count);

      // Tells the root swing node in the comm to spawn a set of cache nodes
      // under it, also creates a mapping of cache nodes to coordinator swing
      // nodes so that they can organize themselves.
      void spawn_cache_nodes(uint32_t job_num, MPI_Comm *parent, uint16_t count);

      // Tells the root swing node in the comm to spawn a count # of job nodes
      // dictated by the exec_name.
      void spawn_job_nodes(uint32_t job_num, std::string exec_name,
            MPI_Comm *parent, uint16_t count);

      // Check for and handle user input from STDIN to add and manage jobs.
      void stdin_select();

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
