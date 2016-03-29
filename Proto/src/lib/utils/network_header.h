#ifndef __NETWORK__HEADER__H__
#define __NETWORK__HEADER__H__

#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <vector>

#define INITIAL_BUF_SIZE 65535
#define MAX_MAPPING_SIZE 32768
#define MAX_KEY_SIZE 4096
#define MAX_VALUE_SIZE 32768
#define MAX_EXEC_NAME_SIZE 255

#define ASSERT_TRUE(expression, todo) {\
    if (!(expression)) {\
        perror("\n!!! ASSERT FAILED !!!\n\tError ");\
        fprintf(stderr, "\tFile : \"%s\"\n\tFunction : \"%s\"\n\t"\
                "Line : %d\n\n", __FILE__, __func__, __LINE__);\
        todo;\
    }\
}

// Enums for different tags (message flags) between a CacheNode and the sender.
enum MsgTag { PUT, PUT_ACK, GET, GET_ACK, PUSH, PUSH_ACK, DROP, DROP_ACK,
              REF, REF_ACK, SPAWN_JOB, SPAWN_CACHE, EXIT };

// Struct to keep track of messages from other nodes which are waiting to be
// tended to.
// tag ----> the tag associated with a message.
// src ----> the src rank of the sender of the message.
// count --> the length of the message in bytes.
// comm ---> the communicator of the sender.
typedef struct MsgInfo {
    uint32_t tag;
    uint32_t src;
    int count;
    MPI_Comm comm;
} __attribute__((packed)) MsgInfo;

// Partial header for the Spawn Job Nodes message.
// job_num -----> the number associated with this job.
// count -------> the number of job nodes to spawn.
// mapping_len -> the length of the mapping array
// mapping -----> the beginning of an array of uin32_t's which maps each job node
//                to its corresponding cache node.
// In the case of spawning job nodes, at the end of the mapping array there is
// the length of the name of the executable to spawn each job node with as well
// as the actual executable name.
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
    uint32_t key_size;
    uint8_t key[MAX_KEY_SIZE];
    uint32_t value_size;
    uint8_t value[MAX_VALUE_SIZE];
    uint64_t timestamp;
} __attribute__((packed)) PutTemplate;

typedef struct PutAckTemplate {
    uint32_t job_num;
    uint32_t job_node;
    uint32_t key_size;
    uint8_t key[MAX_KEY_SIZE];
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
} __attribute__((packed)) GetAckTemplate;

typedef struct CensusTemplate {
    uint32_t job_num;
    uint32_t job_node;
    uint32_t key_size;
    uint8_t key[MAX_KEY_SIZE];
    uint32_t votes_req;
} __attribute__((packed)) CensusTemplate;

// Struct to keep track of which communicators are associated with a given job
// number.
// swing -> the swing node communicator for this job
// cache -> the cache node communicator for this job
// job ---> the job node communicator for this job
typedef struct CommGroup {
    MPI_Comm swing;
    MPI_Comm cache;
    MPI_Comm job;
} __attribute__((packed)) Comms;

typedef struct JobNodeID {
    uint32_t job_num;
    uint32_t job_node;

    bool operator== (const JobNodeID& other) {
        return this->job_num == other.job_num &&
            this->job_node == other.job_node;
    }
} __attribute__((packed)) JobNodeID;

typedef struct Parcel {
    uint64_t timestamp;
    std::string value;

    bool operator< (const Parcel& other) {
        return this->timestamp < other.timestamp;
    }
} __attribute((packed)) Parcel;

void print_msg_info(MsgInfo *msg_info);

void print_msg_tag_handle(MsgTag tag);

void vector_to_stringlist(std::vector<char> &vec, std::string &result);
void stringlist_to_vector(std::vector<char> &vec, std::string &result);

void vector_to_stringlist(std::vector<uint32_t> &vec, std::string &result);
void stringlist_to_vector(std::vector<uint32_t> &vec, std::string &result);

void replace_commas(std::string &str);
void remove_commas(std::vector<char> &vec);

// Sends a message in a non-blocking way, but ensures that the contents of the
// message is buffered into the network before returning. In this way it ensures
// that the request object can be reused.
void send_msg(const void *buf, int count, MPI_Datatype datatype, int dest,
        int tag, MPI_Comm comm, MPI_Request *request);

// Recvs a message in a blocking-way.
void recv_msg(void *buf, int count, MPI_Datatype datatype, int source, int tag,
        MPI_Comm comm, MPI_Status *status);

// Loops until the message content associated with the request object has been
// successfully buffered into the network, at which point the request object is
// deallocated and the call returns.
void wait_for_send(MPI_Request *request);

void get_timestamp(uint64_t *timestamp);

#endif
