#include <stdio.h>
#include <mpi.h>
#include "cache_api/cache_api.h"
#include "network/network.h"
#include "utils/utils.h"

int main(int argc, char **argv) {
   MPI_Init(&argc, &argv);

   Cache cache(&argc, &argv);

   int local_rank;
   MPI_Comm_rank(MPI_COMM_WORLD, &local_rank);

   if (local_rank == 0) {
      cache.put_local("Watson", "Corgi");    
      printf("jobe_node %d - cache.put -> %s/%s returned!\n", local_rank, "Watson",
            "Corgi");
   }

   if (local_rank == 1) {
      cache.put_local("Hedgehog", "Cute");    
      printf("jobe_node %d - cache.put -> %s/%s returned!\n", local_rank, "Hedgehog",
            "Cute");
   }

   MPI_Barrier(MPI_COMM_WORLD);

   if (local_rank == 2) {
      cache.put_local("Watson", "Doof");    
      printf("jobe_node %d - cache.put -> %s/%s returned!\n", local_rank, "Watson",
            "Doof");
   }

   MPI_Barrier(MPI_COMM_WORLD);

   if (local_rank == 0) {
      std::string value;
      cache.get_local("Watson", value);
      printf("job_node %d - cache.get_local -> %s/%s\n", local_rank, "Watson", value.c_str());
      cache.get("Watson", value);
      printf("job_node %d - cache.get -> %s/%s\n", local_rank, "Watson", value.c_str());
      cache.get("Hedgehog", value);
      printf("job_node %d - cache.get -> %s/%s\n", local_rank, "Hedgehog", value.c_str());
      cache.get("Fail", value);
      printf("job_node %d - cache.get -> %s/%s\n", local_rank, "Fail", value.c_str());
   }

   MPI_Finalize();

   return 0;
}
