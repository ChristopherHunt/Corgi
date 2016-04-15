#include <mpi.h>
#include "leader_node/leader_node.h"

int main(int argc, char **argv) {
   MPI_Init(NULL, NULL);

   ASSERT(argc == 2, MPI_Abort(MPI_COMM_WORLD, 1));
   std::string job_exec = argv[1];

   LeaderNode leader_node(job_exec);

   MPI_Finalize();

   return 0;
}
