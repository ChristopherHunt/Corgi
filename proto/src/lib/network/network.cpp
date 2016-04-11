#include <stdio.h>
#include "network.h"

void print_msg_info(MsgInfo *msg_info) {
   MsgTag tag = (MsgTag)msg_info->tag;
   printf("===== MsgInfo =====\n");
   printf("tag ---------> %s\n", msg_tag_handle(tag));
   printf("src ---------> %d\n", msg_info->src);
   printf("count (bytes): %d\n", msg_info->count);

   if (msg_info->comm == MPI_COMM_WORLD) {
      printf("comm: MPI_COMM_WORLD\n");
   }
   else {
      printf("comm: OTHER\n");
   }
}

const char * msg_tag_handle(MsgTag tag) {
   switch (tag) {
      case PUT:
         return "PUT";
      case PUT_ACK:
         return "PUT_ACK";
      case PUT_LOCAL:
         return "PUT_LOCAL";
      case PUT_LOCAL_ACK:
         return "PUT_LOCAL_ACK";
      case GET:
         return "GET";
      case GET_ACK:
         return "GET_ACK";
      case GET_LOCAL:
         return "GET_LOCAL";
      case GET_LOCAL_ACK:
         return "GET_LOCAL_ACK";
      case PUSH:
         return "PUSH";
      case PUSH_ACK:
         return "PUSH_ACK";
      case DROP:
         return "DROP";
      case DROP_ACK:
         return "DROP_ACK";
      case SPAWN_JOB:
         return "SPAWN_JOB";
      case SPAWN_CACHE:
         return "SPAWN_CACHE";
      case EXIT:
         return "EXIT";
      default:
         return "UNKNOWN FLAG!";
   }
}

int send_msg(const void *buf, int count, MPI_Datatype datatype, int dest,
      int tag, MPI_Comm comm, MPI_Request *request) {
#ifdef DEBUG
   printf("send_msg:\n");
   printf("\ttag: ");
   print_msg_tag_handle((MsgTag)tag);
   printf("\n");
   printf("\tdest: %d\n", dest);
#endif

   int result;

   result = MPI_Isend(buf, count, datatype, dest, tag, comm, request);
   wait_for_send(request);
   return result;
}

int recv_msg(void *buf, int count, MPI_Datatype datatype, int source, int tag,
      MPI_Comm comm, MPI_Status *status) {
   return MPI_Recv(buf, count, datatype, source, tag, comm, status);
}

void wait_for_send(MPI_Request *request) {
   int flag = 0;
   MPI_Status status;
   while (flag == 0) {
      MPI_Test(request, &flag, &status);
   }
}
