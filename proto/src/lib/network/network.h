#ifndef __NETWORK__H__
#define __NETWORK__H__

#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/select.h>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

#define SUCCESS 1
#define FAILURE 0

#define STDIN 0
#define KILL_ALL_JOBS -1

#define INITIAL_BUF_SIZE 65535
#define MAX_MAPPING_SIZE 32768
#define MAX_KEY_SIZE 4096
#define MAX_VALUE_SIZE 32768
#define MAX_EXEC_NAME_SIZE 255

// Enums for different tags (message flags) between a CacheNode and the sender.
enum MsgTag { PUT, PUT_ACK, PUT_LOCAL, PUT_LOCAL_ACK, GET, GET_ACK, GET_LOCAL,
   GET_LOCAL_ACK, PUSH, PUSH_ACK, PUSH_LOCAL, PUSH_LOCAL_ACK, PULL,
   PULL_ACK, PULL_LOCAL, PULL_LOCAL_ACK, DROP, DROP_ACK, DROP_LOCAL,
   DROP_LOCAL_ACK, SPAWN_JOB, SPAWN_CACHE, EXIT, EXIT_ACK };

typedef struct MsgInfo {
   uint32_t tag;
   uint32_t src;
   int count;
   MPI_Comm comm;
} __attribute__((packed)) MsgInfo;

typedef struct SpawnNodesTemplate {
   uint32_t job_num;
   uint16_t count;
   uint16_t mapping_size;
   uint8_t mapping[MAX_MAPPING_SIZE];
   uint8_t exec_size;
   uint8_t exec_name[MAX_EXEC_NAME_SIZE];
} __attribute__((packed)) SpawnNodesTemplate;

typedef struct PutTemplate {
   uint32_t job_num;
   uint32_t job_node;
   uint32_t cache_node;
   uint32_t key_size;
   uint8_t key[MAX_KEY_SIZE];
   uint32_t value_size;
   uint8_t value[MAX_VALUE_SIZE];
   uint64_t timestamp;
} __attribute__((packed)) PutTemplate;

typedef struct PutAckTemplate {
   uint32_t job_num;
   uint32_t job_node;
   uint32_t cache_node;
   uint32_t key_size;
   uint8_t key[MAX_KEY_SIZE];
   uint8_t result;
} __attribute__((packed)) PutAckTemplate;

typedef struct GetTemplate {
   uint32_t job_num;
   uint32_t job_node;
   uint32_t key_size;
   uint8_t key[MAX_KEY_SIZE];
   uint64_t timestamp;
} __attribute__((packed)) GetTemplate;

typedef struct GetAckTemplate {
   uint32_t job_num;
   uint32_t job_node;
   uint32_t key_size;
   uint8_t key[MAX_KEY_SIZE];
   uint32_t value_size;
   uint8_t value[MAX_VALUE_SIZE];
   uint64_t timestamp;
   uint8_t result;
} __attribute__((packed)) GetAckTemplate;

typedef struct CensusTemplate {
   uint32_t job_num;
   uint32_t job_node;
   uint32_t key_size;
   uint8_t key[MAX_KEY_SIZE];
   uint32_t votes_req;
} __attribute__((packed)) CensusTemplate;

typedef struct PushTemplate {
   uint32_t job_num;
   uint32_t job_node;
   uint32_t cache_node;
   uint32_t key_size;
   uint8_t key[MAX_KEY_SIZE];
} __attribute__((packed)) PushTemplate;

typedef struct PushAckTemplate {
   uint32_t job_num;
   uint32_t job_node;
   uint32_t cache_node;
   uint32_t key_size;
   uint8_t key[MAX_KEY_SIZE];
   uint8_t result;
} __attribute__((packed)) PushAckTemplate;

// If job_num == KILL_ALL_JOBS then kill all nodes.
typedef struct ExitTemplate {
   int32_t job_num;
} __attribute__((packed)) ExitTemplate;

class SpawnNodesTemp {
   public:
      uint32_t job_num;
      uint16_t count;
      uint16_t mapping_size;
      std::string mapping;
      uint8_t exec_size;
      std::string exec_name;

      SpawnNodesTemp();

      ~SpawnNodesTemp();

      void pack(SpawnNodesTemplate *temp, uint32_t job_num, uint16_t count,
            const std::string& mapping, const std::string& exec_name);

      void unpack(SpawnNodesTemplate *temp);
};

class PutTemp {
   public:
      uint32_t job_num;
      uint32_t job_node;
      uint32_t cache_node;
      uint32_t key_size;
      std::string key;
      uint32_t value_size;
      std::string value;
      uint64_t timestamp;

      PutTemp();

      ~PutTemp();

      void pack(PutTemplate *temp, uint32_t job_num, uint32_t job_node,
            uint32_t cache_node, const std::string& key, const std::string& value,
            uint64_t timestamp);

      void unpack(PutTemplate *temp);
};

class PutAckTemp {
   public:
      uint32_t job_num;
      uint32_t job_node;
      uint32_t cache_node;
      uint32_t key_size;
      std::string key;
      uint8_t result;

      PutAckTemp();

      ~PutAckTemp();

      void pack(PutAckTemplate *temp, uint32_t job_num, uint32_t job_node,
            uint32_t cache_node, const std::string& key, uint8_t result);

      void unpack(PutAckTemplate *temp);
};

class GetTemp {
   public:
      uint32_t job_num;
      uint32_t job_node;
      uint32_t key_size;
      std::string key;
      uint64_t timestamp;

      GetTemp();

      ~GetTemp();

      void pack(GetTemplate *temp, uint32_t job_num, uint32_t job_node,
            const std::string& key, uint64_t timestamp);

      void unpack(GetTemplate *temp);
};

class GetAckTemp {
   public:
      uint32_t job_num;
      uint32_t job_node;
      uint32_t key_size;
      std::string key;
      uint32_t value_size;
      std::string value;
      uint64_t timestamp;
      uint8_t result;

      GetAckTemp();

      ~GetAckTemp();

      void pack(GetAckTemplate *temp, uint32_t job_num, uint32_t job_node,
            const std::string& key, const std::string& value, uint64_t timestamp,
            uint8_t result);

      void unpack(GetAckTemplate *temp);
};

class CensusTemp {
   public:
      uint32_t job_num;
      uint32_t job_node;
      uint32_t key_size;
      std::string key;
      uint32_t votes_req;

      CensusTemp();

      ~CensusTemp();

      void pack(CensusTemplate *temp, uint32_t job_num, uint32_t job_node,
            const std::string& key, uint32_t votes_req);

      void unpack(CensusTemplate *temp);
};

class PushTemp {
   public:
      uint32_t job_num;
      uint32_t job_node;
      uint32_t cache_node;
      uint32_t key_size;
      std::string key;

      PushTemp();

      ~PushTemp();

      void pack(PushTemplate *temp, uint32_t job_num, uint32_t job_node,
            uint32_t cache_node, const std::string& key);

      void unpack(PushTemplate *temp);
};

class PushAckTemp {
   public:
      uint32_t job_num;
      uint32_t job_node;
      uint32_t cache_node;
      uint32_t key_size;
      std::string key;
      uint8_t result;

      PushAckTemp();

      ~PushAckTemp();

      void pack(PushAckTemplate *temp, uint32_t job_num, uint32_t job_node,
            uint32_t cache_node, const std::string& key, uint8_t result);

      void unpack(PushAckTemplate *temp);
};

// Prints the info associated with a message to stdout in a formatted way.
void print_msg_info(MsgInfo *msg_info);

// Returns the label associated with a given numeric message tag to stdout.
const char * msg_tag_handle(MsgTag tag);

// Sends a message in a non-blocking way, but ensures that the contents of the
// message is buffered into the network before returning. In this way it ensures
// that the request object can be reused when then function returns. Returns the
// return value of the internal MPI_ISend call.
int send_msg(const void *buf, int count, MPI_Datatype datatype, int dest,
      int tag, MPI_Comm comm, MPI_Request *request);

// Recvs a message in a blocking-way. Returns the return value of the internal
// MPI_Recv call.
int recv_msg(void *buf, int count, MPI_Datatype datatype, int source, int tag,
      MPI_Comm comm, MPI_Status *status);

// Loops until the message content associated with the request object has been
// successfully buffered into the network, at which point the request object is
// deallocated and the call returns.
void wait_for_send(MPI_Request *request);

#endif
