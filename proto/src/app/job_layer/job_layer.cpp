#include <stdio.h>
#include <mpi.h>
#include "cache_api/cache_api.h"

int main(int argc, char **argv) {
   MPI_Init(&argc, &argv);
   Cache *cache = new Cache(&argc, &argv);

   int local_rank;
   std::string value;
   MPI_Comm_rank(MPI_COMM_WORLD, &local_rank);

   if (local_rank == 0) {
      cache->put_local("Watson", "Corgi");    
      printf("job_node %d - cache->put_local -> %s/%s returned!\n", local_rank, "Watson",
            "Corgi");
   }

   MPI_Barrier(MPI_COMM_WORLD);

   if (local_rank == 1) {
      //cache->get("Watson", value);
      //printf("job_node %d - cache->get -> %s/%s returned!\n", local_rank, "Watson",
            //value.c_str());
   }

   if (local_rank == 1) {
      cache->put_local("Hedgehog", "Cute");    
      printf("jobe_node %d - cache->put_local -> %s/%s returned!\n", local_rank, "Hedgehog",
            "Cute");
   }

   MPI_Barrier(MPI_COMM_WORLD);

   if (local_rank == 2) {
      cache->put_local("Watson", "Doof");    
      printf("jobe_node %d - cache->put_local -> %s/%s returned!\n", local_rank, "Watson",
            "Doof");
   }

   MPI_Barrier(MPI_COMM_WORLD);

   if (local_rank == 0) {
      cache->get_local("Watson", value);
      printf("job_node %d - cache->get_local -> %s/%s\n", local_rank, "Watson", value.c_str());
      //cache->get("Watson", value);
      //printf("job_node %d - cache->get -> %s/%s\n", local_rank, "Watson", value.c_str());
      //cache->get("Hedgehog", value);
      //printf("job_node %d - cache->get -> %s/%s\n", local_rank, "Hedgehog", value.c_str());
      //cache->get("Fail", value);
      //printf("job_node %d - cache->get -> %s/%s\n", local_rank, "Fail", value.c_str());
   }

   if (local_rank == 0) {
      printf("jobe_node %d - calling push_local!\n", local_rank);
      cache->push_local("Watson", 1);
      printf("jobe_node %d - cache->push_local -> %s/%s to node %d returned!\n",
         local_rank, "Watson", "Corgi", 1);
   }

   MPI_Barrier(MPI_COMM_WORLD);

   if (local_rank == 1) {
      cache->get_local("Watson", value);
      printf("job_node %d - cache->get_local -> %s/%s\n", local_rank, "Watson", value.c_str());
   }

   MPI_Barrier(MPI_COMM_WORLD);

   if (local_rank == 0) {
      //cache->get("Watson", value);
      //printf("job_node %d - cache->get -> %s/%s\n", local_rank, "Watson", value.c_str());
      //cache->get("Hedgehog", value);
      //printf("job_node %d - cache->get -> %s/%s\n", local_rank, "Hedgehog", value.c_str());
      //cache->get("Fail", value);
      //printf("job_node %d - cache->get -> %s/%s\n", local_rank, "Fail", value.c_str());
   }

   delete(cache);

   MPI_Barrier(MPI_COMM_WORLD);

   MPI_END_TEST();

   return 0;
}
