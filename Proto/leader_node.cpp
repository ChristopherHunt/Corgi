#include <stdlib.h>
#include <string.h>
#include "leader_node.h"

LeaderNode::LeaderNode() {
    // Set tag counter to 0;
    next_tag = 0;

    // Determine where this node is in the system.
    orient();

    // Allocate space for data structures within this object.
    //allocate();

    // TODO: REMOVE THIS (just for testing)
    create_test_job();

    // Handle all requests sent to this cache node.
    handle_requests();
}

LeaderNode::~LeaderNode() {
    //free(buf);
}

/*
void LeaderNode::allocate() {
    buf = (uint8_t *)calloc(INITIAL_BUF_SIZE, sizeof(uint8_t));
    ASSERT_TRUE(buf != NULL, MPI_Abort(MPI_COMM_WORLD, 1));
}
*/

/*
void LeaderNode::define_datatypes() {
    MPI_Type_contiguous(2, MPI_INT, &PAIR);
    MPI_Type_commit(&PAIR);
}
*/

// TODO: REMOVE THIS METHOD (It is just for testing functionality).
void LeaderNode::create_test_job() {
    MPI_Comm temp;

    // TODO: This is a simple place holder for swing node spawning,
    //       will want to make this more flexible later.
    spawn_swing_nodes(MPI_COMM_WORLD, &temp, 4);

    int tag = next_tag++;

    name_to_tag["Job1"] = tag;
    tag_to_comm[tag] = temp;

    // TODO: REMOVE
    int size;
    MPI_Comm_remote_size(tag_to_comm[tag], &size);
    printf("after --- Job1 size (swing node count): %d\n", size);
    //

    // TODO: This is a simple place holder for cache node spawning,
    //       will want to make this more flexible later.
    spawn_cache_nodes(tag, &temp, 4);

    // TODO: Make a better way of adding mappings for coordinator nodes.
    //       Use this bandaid to get off the ground for now.
    std::map<int, int> temp_map;
    temp_map[0] = 0;
    temp_map[1] = 0;
    temp_map[2] = 1;
    temp_map[3] = 1;
    tag_to_coord[tag] = temp_map;

    // TODO: Make a better way of adding mappings for team nodes.
    //       Use this bandaid to get off the ground for now.
    temp_map.clear();
    temp_map[0] = 0;
    temp_map[1] = 1;
    temp_map[2] = 2;
    temp_map[3] = 3;
    tag_to_team[tag] = temp_map;
}

void LeaderNode::handle_coord_query() {
    printf("===== COORD QUERY =====\n");
    printf("LeaderNode %d\n", local_rank);
    print_msg_info();

    ASSERT_TRUE(msg_info.count / sizeof(int) == 2, MPI_Abort(MPI_COMM_WORLD, 1));

    MPI_Recv(pair, 2, MPI_INT, msg_info.src, msg_info.tag, msg_info.comm,
        &status);

    int job_tag = pair[0];
    int cache_node = pair[1];
    int coord_node = tag_to_coord[job_tag][cache_node];

    triple[0] = job_tag;
    triple[1] = coord_node;
    triple[2] = cache_node;

    printf("Leader responding to COORD_QUERY!\n");
    MPI_Send(triple, 3, MPI_INT, msg_info.src, COORD_QUERY_ACK, msg_info.comm);
}

void LeaderNode::handle_team_query() {
    printf("===== TEAM QUERY =====\n");
    printf("LeaderNode %d\n", local_rank);
    print_msg_info();
}

void LeaderNode::handle_spawn_job() {
    printf("===== SPAWN JOB =====\n");
    printf("LeaderNode %d\n", local_rank);
    print_msg_info();
}

void LeaderNode::handle_spawn_cache() {
    printf("===== SPAWN CACHE =====\n");
    printf("LeaderNode %d\n", local_rank);
    print_msg_info();
}

void LeaderNode::handle_exit() {
    printf("===== EXIT =====\n");
    printf("LeaderNode %d\n", local_rank);
    print_msg_info();
}

void LeaderNode::handle_requests() {
    printf("LeaderNode entering handle_requests!\n");
    while (true) {
        while (msg_ready() == false) {
            message_select();
        }

        while (msg_ready() == true) {
            msg_info = msg_queue.front();
            msg_queue.pop();
            printf("msg_queue.size: %d\n", msg_queue.size());

            switch (msg_info.tag) {
                case COORD_QUERY:
                    handle_coord_query();
                    break;

                case TEAM_QUERY:
                    handle_team_query();
                    break;

                case SPAWN_JOB:
                    handle_spawn_job();
                    break;

                case SPAWN_CACHE:
                    handle_spawn_cache();
                    break;

                case EXIT:
                    handle_exit();
                    break;

                default:
                    printf("===== DEFAULT =====\n");
                    printf("LeaderNode %d\n", local_rank);
                    print_msg_info();
                    ASSERT_TRUE(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
                    break;
            }
        }
    }
}

void LeaderNode::message_select() {
    int flag;

    for (auto const &entry : tag_to_comm) { 
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, entry.second, &flag, &status);

        if (flag == 1) {
            msg_info.tag = status.MPI_TAG; 
            msg_info.src = status.MPI_SOURCE;
            msg_info.comm = entry.second;
            MPI_Get_count(&status, MPI_BYTE, &msg_info.count);
            msg_queue.push(msg_info);
        }
    }
}

bool LeaderNode::msg_ready() {
    return msg_queue.size() > 0 ? true : false;
}

void LeaderNode::orient() {
    // Get data on local comm
    MPI_Comm_size(MPI_COMM_WORLD, &local_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &local_rank);

    if (local_size != 1) {
        printf("Currently do not support multiple leader nodes, exiting!\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
}

void LeaderNode::print_msg_info() {
    printf("===== MsgInfo =====\n");
    printf("tag ---------> %d\n", msg_info.tag);
    printf("src ---------> %d\n", msg_info.src);
    printf("count (bytes): %d\n", msg_info.count);

    if (msg_info.comm == MPI_COMM_WORLD) {
        printf("comm:  MPI_COMM_WORLD\n");
    }
    else {
        printf("comm:  OTHER\n");
    }
}

void LeaderNode::spawn_swing_nodes(MPI_Comm parent, MPI_Comm *child, int count) {
    // TODO: Make it so we can get unique comm handles prior to placing them in
    //       the swing comm queue. For now just hardcode a name to make things
    //       easier for testing.
    MPI_Comm_dup(parent, child);

    MPI_Comm_spawn("swing_node_main", MPI_ARGV_NULL, count, MPI_INFO_NULL, 0,
        *child, child, MPI_ERRCODES_IGNORE);
}

void LeaderNode::spawn_cache_nodes(int job_tag, MPI_Comm *comm, int count) {
    printf("LeaderNode sending SPAWN_CACHE of size %d\n", count);
    int comm_size;
    MPI_Comm_remote_size(*comm, &comm_size);

    pair[0] = job_tag;
    pair[1] = count;

    // TODO: Look into MPI_Comm_Idup and perhaps MPI_Bcast for sending out this
    //       spawn request to all nodes efficiently and having them all handle it
    //       efficiently.
    for (int i = 0; i < comm_size; ++i) {
        printf("Leader sending spawn cache msg to swing node %d\n", i);
        MPI_Send(pair, 2, MPI_INT, i, SPAWN_CACHE, *comm);
    }
}
