#include <stdio.h>
#include "network.h"

void print_msg_info(MsgInfo *msg_info) {
   MsgTag tag = (MsgTag)msg_info->tag;
   printf("===== MsgInfo =====\n");
   printf("tag ---------> ");
   print_msg_tag_handle(tag);
   printf("\n");
   printf("src ---------> %d\n", msg_info->src);
   printf("count (bytes): %d\n", msg_info->count);

   if (msg_info->comm == MPI_COMM_WORLD) {
      printf("comm: MPI_COMM_WORLD\n");
   }
   else {
      printf("comm: OTHER\n");
   }
}

void print_msg_tag_handle(MsgTag tag) {
   switch (tag) {
      case PUT:
         printf("PUT");
         break;
      case PUT_ACK:
         printf("PUT_ACK");
         break;
      case GET:
         printf("GET");
         break;
      case GET_ACK:
         printf("GET_ACK");
         break;
      case PUSH:
         printf("PUSH");
         break;
      case PUSH_ACK:
         printf("PUSH_ACK");
         break;
      case DROP:
         printf("DROP");
         break;
      case DROP_ACK:
         printf("DROP_ACK");
         break;
      case SPAWN_JOB:
         printf("SPAWN_JOB");
         break;
      case SPAWN_CACHE:
         printf("SPAWN_CACHE");
         break;
      case EXIT:
         printf("EXIT");
         break;
      default:
         printf("UNKNOWN FLAG!");
         break;
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
