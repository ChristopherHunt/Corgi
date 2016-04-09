#include <mpi.h>
#include "network/network.h"
#include "swing_node/swing_node.h"
#include "utils/utils.h"

int main(int argc, char **argv) {
   MPI_Init(NULL, NULL);

   SwingNode swing_node;

   MPI_Finalize();

   return 0;
}
