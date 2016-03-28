#include <mpi.h>
#include "leader_node.h"

int main(int argc, char **argv) {
    MPI_Init(NULL, NULL);

    LeaderNode leader_node;

    MPI_Finalize();

    return 0;
}
