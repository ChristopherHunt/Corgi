#ifndef __NETWORK__HEADER__H__
#define __NETWORK__HEADER__H__

#include <stdint.h>
#include <stdio.h>

#define INITIAL_BUF_SIZE 1024

// Enums for different tags (message flags) between a CacheNode and the sender.
enum MsgFlags { CONNECT, PUT, PUT_ACK, GET, GET_ACK, FORWARD, DELETE,
                DELETE_ACK, COORD_QUERY, COORD_QUERY_ACK, TEAM_QUERY,
                SPAWN_JOB, SPAWN_CACHE, EXIT };

// Struct to keep track of messages from other nodes which are waiting to be
// tended to.
typedef struct MsgInfo {
    int tag;
    int src;
    int count;
    MPI_Comm comm;
} __attribute__((packed)) MsgInfo;

// Struct used to pass an int around with the job its associated with.
typedef struct ValName {
    int val;
    uint8_t name;
} __attribute__((packed)) ValName;

#define ASSERT_TRUE(expression, todo) {\
   if (!(expression)) {\
      perror("\n!!! ASSERT FAILED !!!\n\tError ");\
      fprintf(stderr, "\tFile : \"%s\"\n\tFunction : \"%s\"\n\t"\
            "Line : %d\n\n", __FILE__, __func__, __LINE__);\
      todo;\
   }\
}

#endif
