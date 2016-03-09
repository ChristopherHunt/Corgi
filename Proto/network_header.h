#ifndef __NETWORK__HEADER__H__
#define __NETWORK__HEADER__H__

#include <stdio.h>

// Enums for different tags (message flags) between a CacheNode and the sender.
enum MsgFlags { CONNECT, PUT, PUT_ACK, GET, GET_ACK, FORWARD, DELETE,
                DELETE_ACK, COORD_QUERY, SPAWN, EXIT };

// Struct to keep track of messages from other nodes which are waiting to be
// tended to.
typedef struct MsgInfo {
    int tag;
    int src;
    int count;
    MPI_Comm comm;
} MsgInfo;

#define ASSERT_TRUE(expression, todo) {\
   if (!(expression)) {\
      perror("\n!!! ASSERT FAILED !!!\n\tError ");\
      fprintf(stderr, "\tFile : \"%s\"\n\tFunction : \"%s\"\n\t"\
            "Line : %d\n\n", __FILE__, __func__, __LINE__);\
      todo;\
   }\
}

#endif
