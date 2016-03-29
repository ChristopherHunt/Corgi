#include <stdio.h>
#include <mpi.h>
#include "../cache_api/cache.h"
#include "../utils/network_header.h"

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    Cache cache(&argc, &argv);

    int local_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &local_rank);

    if (local_rank == 1) {
        cache.put("Watson", "Corgi");    
        printf("jobe_node %d - cache.put -> %s returned!\n", local_rank, "Watson");
    }

    if (local_rank == 3) {
        cache.put("Hedgehog", "Cute");    
        printf("jobe_node %d - cache.put -> %s returned!\n", local_rank, "Hedgehog");
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if (local_rank == 2) {
        cache.put("Watson", "Doof");    
        printf("jobe_node %d - cache.put -> %s returned!\n", local_rank, "Watson");
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if (local_rank == 0) {
        std::string value;
        cache.get("Watson", value);
        printf("job_node %d - cache.get -> %s/%s\n", local_rank, "Watson", value.c_str());
        cache.get("Hedgehog", value);
        printf("job_node %d - cache.get -> %s/%s\n", local_rank, "Hedgehog", value.c_str());
        cache.get("Fail", value);
        printf("job_node %d - cache.get -> %s/%s\n", local_rank, "Fail", value.c_str());
    }
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
