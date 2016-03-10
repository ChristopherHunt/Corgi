#ifndef __CACHE__NODE__H__
#define __CACHE__NODE__H__

#include <iostream>
#include <map>
#include <mpi.h>
#include <queue>
#include "network_header.h"

class CacheNode {
    public:
        CacheNode();

        ~CacheNode();

    private:
        MPI_Status status;      // Status structure for checking communications.
        MPI_Comm parent_comm;   // Intercommunicator between local & parent comm
        int parent_size;        // Size of parent comm
        int parent_rank;        // Rank of this node in parent comm
        int local_size;         // Size of MPI_COMM_WORLD
        int local_rank;         // Rank of this node in MPI_COMM_WORLD
        int coord_rank;         // Rank of coord swing node in parent comm.

        // Struct to hold info about new messages.
        MsgInfo msg_info;

        // Queue of messages for the cache_node to handle.
        std::queue<MsgInfo> msg_queue;

        // Map of keys to values on this local node.
        // TODO: Decide if we want to use a map or an unordered_map here
        std::map<std::string, std::string> cache;

        // Handle the case where another node wants to connect to this node's
        // TCP socket (TODO: NOT SURE IF THIS IS FLAG IS NEEDED, NEED TO READ
        // MORE).
        void handle_connect();

        void handle_coord_query_ack();

        void handle_put();

        void handle_put_ack();

        void handle_get();

        void handle_get_ack();

        void handle_forward();

        void handle_delete();

        void handle_delete_ack();

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
        void print_msg_info();

        // Creates TCP socket for Job nodes to talk with this cache node. Also
        // communicates this information with its coordinator swing node.
        void setup_socket();
};

#endif

