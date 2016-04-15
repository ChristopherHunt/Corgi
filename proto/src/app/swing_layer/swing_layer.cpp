#include <mpi.h>
#include "swing_node/swing_node.h"

int main(int argc, char **argv) {
   MPI_Init(NULL, NULL);

   SwingNode swing_node;

   MPI_Finalize();

   return 0;
}
