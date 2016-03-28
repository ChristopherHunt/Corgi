#ifndef __LEADER__NODE__H__
#define __LEADER__NODE__H__

#include <map>
#include <mpi.h>
#include <deque>
#include <set>
#include <vector>
#include <stdint.h>
#include <string>
#include "../utils/network_header.h"

class LeaderNode {
    public:
        LeaderNode();

        ~LeaderNode();

    private:
        MPI_Status status;      // Status structure for checking communications.

        int local_size;         // Size of MPI_COMM_WORLD
        int local_rank;         // Rank of this node in MPI_COMM_WORLD
        uint32_t next_job_num;  // A counter for assigning new job tags.

        // Struct to hold info about new messages.
        MsgInfo msg_info;

        // Buffer for holding message data.
        uint8_t *buf;

        // Queue of messages for the leader_node to handle.
        std::deque<MsgInfo> msg_queue;

        // Map of job_num to swing node communicator.
        std::map<uint32_t, MPI_Comm> job_to_comm;

        // Map of job tag to map of cache node -> coord swing node.
        std::map<uint32_t, std::vector<uint32_t> > job_to_swing;

        // Map of job tag to team node pairings (job node -> cache_node).
        std::map<uint32_t, std::vector<uint32_t> > job_to_cache;

        // Allocates space for dynamic data structures within the object.
        void allocate();

        // Method which creates a hard-coded job for cache testing.
        void create_test_job();

        // Defines the MPI_Datatypes needed for this node to communicate with
        // others.
        void define_datatypes();

        void handle_team_query();

        void handle_spawn_job();

        void handle_spawn_cache();

        void handle_exit();

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
