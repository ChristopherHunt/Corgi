#include <stdio.h>
#include <mpi.h>
#include <string>
#include "cache_api/cache_api.h"
#include "utility/utils/utils.h"

int main(int argc, char **argv) {
   MPI_Init(&argc, &argv);

   bool success;
   int local_rank;
   std::string value;
   Cache *cache = new Cache(&argc, &argv);

   MPI_Comm_rank(MPI_COMM_WORLD, &local_rank);

   if (local_rank == 0) {
      success = cache->put_local("Watson", "Corgi");
      MPI_ASSERT_EQ(success, true);
   }

   MPI_Barrier(MPI_COMM_WORLD);

   if (local_rank == 0) {
      success = cache->get_local("Watson", value);
      MPI_ASSERT_EQ(success, true);
      MPI_ASSERT_STREQ("Corgi", value.c_str());
   }
   else {
      success = cache->get_local("Watson", value);
      MPI_ASSERT_EQ(success, false);
      MPI_ASSERT_STREQ("", value.c_str());
   }

   delete(cache);

   fprintf(stderr, "Test Node %d finished successfully!\n", local_rank);
   MPI_Barrier(MPI_COMM_WORLD);
   MPI_END_TEST();

   return 0;
}
