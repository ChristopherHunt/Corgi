#ifndef __NODE__H__
#define __NODE__H__

#include <deque>
#include <mpi.h>
#include <unordered_map>
#include "network/network.h"
#include "utils/utils.h"

class Node {
   public:
      MPI_Status status;      // Status structure for checking communications.
      MPI_Comm parent_comm;   // Intercommunicator between local & parent comm
      int parent_size;        // Size of parent comm
      int parent_rank;        // Rank of this node in parent comm
      int local_size;         // Size of MPI_COMM_WORLD
      int local_rank;         // Rank of this node in MPI_COMM_WORLD
      int coord_rank;         // Rank of coord swing node in parent comm.

      // Map of job_num to CommGroup struct containing the swing, cache and
      // job communicators that are bound to this job_num.
      std::unordered_map<uint32_t, CommGroup> job_to_comms;

      // Eventually want to remove this from this header file and incorporate it
      // into only the node classes that need it.
      std::unordered_map<std::string, std::vector<JobNodeID> > key_to_nodes;

      // Struct to hold info about new messages.
      MsgInfo msg_info;

      // Buffer for holding message data.
      uint8_t *buf;

      // Queue of messages for the node to handle.
      std::deque<MsgInfo> msg_queue;
};

#endif
