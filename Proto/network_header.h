#ifndef __NETWORK__HEADER__H__
#define __NETWORK__HEADER__H__

#include <stdint.h>
#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <mpi.h>

#define INITIAL_BUF_SIZE 65535
#define MAX_MAPPING_SIZE 32768
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
enum MsgTag { CONNECT, PUT, PUT_ACK, GET, GET_ACK, FORWARD, DELETE,
              DELETE_ACK, COORD_QUERY, COORD_QUERY_ACK, TEAM_QUERY,
              SPAWN_JOB, SPAWN_CACHE, EXIT };

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

void print_msg_info(MsgInfo *msg_info);

void print_msg_tag_handle(MsgTag tag);

void vector_to_stringlist(std::vector<char> &vec, std::string &result);
void stringlist_to_vector(std::vector<char> &vec, std::string &result);

void vector_to_stringlist(std::vector<uint32_t> &vec, std::string &result);
void stringlist_to_vector(std::vector<uint32_t> &vec, std::string &result);

void replace_commas(std::string &str);
void remove_commas(std::vector<char> &vec);

#endif
