#ifndef __LEADER__NODE__H__
#define __LEADER__NODE__H__

#include <iostream>
#include <map>
#include <mpi.h>
#include <queue>
#include <set>
#include "network_header.h"

class LeaderNode {
    public:
        LeaderNode();

        ~LeaderNode();

    private:
        MPI_Status status;      // Status structure for checking communications.
        int local_size;         // Size of MPI_COMM_WORLD
        int local_rank;         // Rank of this node in MPI_COMM_WORLD

        // Struct to hold info about new messages.
        MsgInfo msg_info;

        // Queue of messages for the cache_node to handle.
        std::queue<MsgInfo> msg_queue;

        // TODO: Probably collapse these two sets into a single set and a
        // strcture in order to make them more tightly coupled.
        MPI_Comm swing_comm;
        MPI_Comm cache_comm;

        // Set of swing node communicators operating under this leader.
        std::set<MPI_Comm> swing_comms;

        // Set of swing node communicators operating under this leader.
        std::set<MPI_Comm> cache_comms;

        // Map of cache nodes to their coordinator swing nodes.
        std::map<int, int> cache_to_coord_swing;

        void handle_coord_query();

        void handle_partner_query();

        void handle_spawn_job();

        void handle_spawn_cache();

        void handle_exit();

        // Handles all message requests from other nodes.
        void handle_requests();

        // Creates a communicator and spawns a set of swing nodes.
        void spawn_swing_nodes(MPI_Comm parent, MPI_Comm *child, int count);

        // Tells a swing node to spawn a set of cache nodes under it, also
        // creates a mapping of cache nodes to coordinator swing nodes so that
        // they can organize themselves.
        void spawn_cache_nodes(MPI_Comm *parent, int count);

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
        void print_msg_info();
};

#endif

