#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include "../utils/network_header.h"
#include "cache.h"

Cache::Cache(int *argc_ptr, char ***argv_ptr) {
    allocate();
    orient(argc_ptr, argv_ptr); 
}

void Cache::allocate() {
   buf = (uint8_t *)calloc(INITIAL_BUF_SIZE, sizeof(uint8_t)); 
   ASSERT_TRUE(buf != NULL, MPI_Abort(MPI_COMM_WORLD, 1));
}

Cache::~Cache() {
    free(buf);
}

void Cache::put(const std::string& key, const std::string& value) {
    PutTemplate *format = (PutTemplate *)buf;
    format->job_num = job_num;
    format->job_node = local_rank;
    format->key_size = key.size();
    memcpy(format->key, key.c_str(), key.size());
    format->value_size = value.size();
    memcpy(format->value, value.c_str(), value.size());
    get_timestamp(&(format->timestamp));

    MPI_Request request;
    MPI_Status status;
    
    printf("Job %d Rank %d calling put on %s/%s!\n", job_num, local_rank, key.c_str(), value.c_str());
    send_msg(buf, sizeof(PutTemplate), MPI_UINT8_T, coord_rank, PUT,
        parent_comm, &request);

    // TODO: FIX THIS ISSUE -- WE COULD HAVE 2 MISALIGNED PUT_ACK RECV'S HERE
    // WHEN WE MOVE TO NON-BLOCKING PUTS. ALSO, THIS TIES UP THE CACHE NODE
    // NEEDLESSLY, SO SHOULD PROBABLY QUEUE UP THE BLOCKING PUT_ACK REQUESTS AND
    // ACT ON THEM AS THEY COME IN.
    recv_msg(buf, sizeof(PutAckTemplate), MPI_UINT8_T, coord_rank, PUT_ACK,
        parent_comm, &status);
}


// Job asks cache for the value associated with a key
// Cache looks to see if it has the key
//  cache node asks swing node to gather the required number of copies from
//  other nodes
// Selected cache nodes send their copy to the asking cache node
// Asking cache node resolves any conflicts using timestamps
// Asking cache node updates its store
// Asking cache node return to the calling job node
void Cache::get(const std::string& key, std::string& value) {
    GetTemplate *format = (GetTemplate *)buf;
    format->job_num = job_num;
    format->job_node = local_rank;
    format->key_size = key.size();
    memcpy(format->key, key.c_str(), key.size());
    get_timestamp(&(format->timestamp));

    MPI_Request request;
    MPI_Status status;
    
    printf("Job %d Rank %d calling get on key %s!\n", job_num, local_rank, key.c_str());
    send_msg(buf, sizeof(GetTemplate), MPI_UINT8_T, coord_rank, GET,
        parent_comm, &request);

    // TODO: FIX THIS ISSUE -- WE COULD HAVE 2 MISALIGNED GET_ACK RECV'S HERE
    // WHEN WE GO TO NON-BLOCKING GET. ALSO, THIS TIES UP THE CACHE NODE
    // NEEDLESSLY, SO SHOULD PROBABLY QUEUE UP THE BLOCKING GET_ACK REQUESTS AND
    // ACT ON THEM AS THEY COME IN.
    recv_msg(buf, sizeof(GetAckTemplate), MPI_UINT8_T, coord_rank, GET_ACK,
        parent_comm, &status);

    GetAckTemplate *temp = (GetAckTemplate *)buf;

    value.clear();
    value.assign((const char *)temp->value, temp->value_size);
}

int32_t Cache::push(const std::string& key, uint32_t node_id) {
    return 0;
}

int32_t Cache::drop(const std::string& key) {
    return 0;
}

int32_t Cache::collect(const std::string& key) {
    return 0;
}

void Cache::get_owners(const std::string& key, std::vector<uint32_t>& owners) {

}

void Cache::orient(int *argc_ptr, char ***argv_ptr) {
    int argc = *argc_ptr;
    char **argv = *argv_ptr;

    ASSERT_TRUE(argc >= 3, MPI_Abort(1, MPI_COMM_WORLD));

    // TODO REMODE
    // Print the argv list for reference
    printf("job_node argc: %d\n", argc);
    for (int i = 0; i < argc; ++i) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
    //
    
    // Get the job_num for this job. 
    char *endptr;
    job_num = strtol(argv[1], &endptr, 10);

    // Grab the job to cache node pairings list. 
    std::string mapping(argv[2]);
    replace_commas(mapping);
    std::vector<uint32_t> map_vec;
    stringlist_to_vector(map_vec, mapping);

    // Update argc and argv so things are transparent to the caller.
    // TODO: Ensure this is working properly
    argc_ptr -= 1;
    std::string exec_name(argv[0]);
    memcpy(argv[2], exec_name.c_str(), exec_name.size());
    *argv_ptr = *(argv_ptr + 2);

    // Get details on the world this node lives in.
    MPI_Comm_size(MPI_COMM_WORLD, &local_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &local_rank);

    MPI_Comm_get_parent(&parent_comm);
    MPI_Comm_remote_size(parent_comm, &parent_size);
    MPI_Comm_rank(parent_comm, &parent_rank);

    // Get coordinator cache node's rank for this job node.
    coord_rank = map_vec[local_rank];

    printf("Job node: local rank - %d/%d parent rank - %d/%d\n", local_rank,
            local_size, parent_rank, parent_size);

    printf("Job Num: %d Job node %d: team cache node: %d\n", job_num, local_rank,
        coord_rank);
}
