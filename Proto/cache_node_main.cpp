#include <mpi.h>
#include "cache_node.h"

int main(int argc, char **argv) {
    MPI_Init(NULL, NULL);

    CacheNode cache_node;

    MPI_Finalize();

    return 0;
}
