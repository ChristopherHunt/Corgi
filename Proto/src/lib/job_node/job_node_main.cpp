#include <stdio.h>
#include <mpi.h>
#include "../cache_api/cache.h"
#include "../utils/network_header.h"

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    Cache cache(&argc, &argv);

    cache.put("key", "value");    

    printf("cache.put returned!\n");
    /*
    printf("job_node argc: %d\n", argc);
    for (int i = 0; i < argc; ++i) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }

    std::string mapping(argv[1]);
    replace_commas(mapping);
    std::vector<uint32_t> map_vec;
    stringlist_to_vector(map_vec, mapping);

    int local_size;
    int local_rank;

    MPI_Comm parent_comm;
    int parent_size;
    int parent_rank;

    MPI_Comm_size(MPI_COMM_WORLD, &local_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &local_rank);

    MPI_Comm_get_parent(&parent_comm);
    MPI_Comm_remote_size(parent_comm, &parent_size);
    MPI_Comm_rank(parent_comm, &parent_rank);

    printf("Job node: local rank - %d/%d parent rank - %d/%d\n", local_rank,
            local_size, parent_rank, parent_size);

    printf("Job node %d: team cache node: %d\n", local_rank, map_vec[local_rank]);
    */

    MPI_Finalize();

    return 0;
}
