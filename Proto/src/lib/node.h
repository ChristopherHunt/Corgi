#ifndef __NODE__H__
#define __NODE__H__

#include <mpi.h>
#include <unordered_map>
#include "utils/network_header.h"

class Node {
    public:
        int coord_rank;
        uint8_t *buf;
        MsgInfo msg_info;
        MPI_Status status;
        MPI_Comm parent_comm;
        int local_rank;
        int local_size;
        int parent_rank;
        int parent_size;

        std::unordered_map<uint32_t, CommGroup> job_to_comms;
        std::unordered_map<std::string, std::vector<JobNodeID> > key_to_nodes;

        /*
        Node();

        ~Node();

        MPI_Status status;      // Status structure for checking communications.
        MPI_Comm parent_comm;   // Intercommunicator between local & parent comm

        int parent_size;        // Size of parent comm
        int parent_rank;        // Rank of this node in parent comm
        int local_size;         // Size of MPI_COMM_WORLD
        int local_rank;         // Rank of this node in MPI_COMM_WORLD

        // Struct to hold info about new messages.
        MsgInfo msg_info;

        // Buffer for holding message data.
        uint8_t *buf;

        // Queue of messages for the node to handle.
        std::queue<MsgInfo> msg_queue;

        // Allocates space for dynamic data structures within the object.
        virtual void allocate() = 0;

        // Cache node asking for its coordinator swing node's rank. Reply with
        // the coordinator swing node's rank.
        void handle_coord_query();

        void handle_coord_query_ack();

        void handle_team_query();

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
        */
};

#endif
