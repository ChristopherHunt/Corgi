#ifndef __SWING__NODE__H__
#define __SWING__NODE__H__

#include <iostream>
#include <map>
#include <mpi.h>
#include <queue>
#include <vector>
#include "network_header.h"

class SwingNode {
    public:
        SwingNode();

        ~SwingNode();

    private:
        MPI_Status status;      // Status structure for checking communications.
        MPI_Comm parent_comm;   // Intercommunicator between local & parent comm
        int parent_size;        // Size of parent comm
        int parent_rank;        // Rank of this node in parent comm
        int local_size;         // Size of MPI_COMM_WORLD
        int local_rank;         // Rank of this node in MPI_COMM_WORLD

        // TODO: This is a temporary comm that is used to get things off the
        //       ground, this will be replaced with a service that creates and
        //       tracks unique comms.
        MPI_Comm cache_comm;

        // Struct to hold info about new messages.
        MsgInfo msg_info;

        // Queue of messages for the cache_node to handle.
        std::queue<MsgInfo> msg_queue;

        // Map of tuple keys to the cache node(s) which contain them.
        // TODO: Decide if we want to use a map or an unordered_map here, as
        //       well as a vector, set, map or unordered_map for the 2nd entry.
        std::map<std::string, std::vector<int> > key_to_node;

        // Cache node asking for its coordinator swing node's rank. Reply with
        // the coordinator swing node's rank.
        void handle_coord_query();

        void handle_partner_query();

        void handle_put();

        void handle_get();

        void handle_delete();

        void handle_delete_ack();

        void handle_forward();

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
        void print_msg_info();
};

#endif

