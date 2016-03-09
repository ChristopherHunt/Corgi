#include "leader_node.h"

LeaderNode::LeaderNode() {
    // Determine where this node is in the system.
    orient();

    // TODO: This is a simple place holder for swing node spawning,
    //       will want to make this more flexible later.
    spawn_swing_nodes(MPI_COMM_WORLD, &swing_comm, 4);

    int size;
    MPI_Comm_remote_size(swing_comm, &size);
    printf("after --- swing_comm size: %d\n", size);

    // TODO: This is a simple place holder for cache node spawning,
    //       will want to make this more flexible later.
    spawn_cache_nodes(&swing_comm, 4);

    // Handle all requests sent to this cache node.
    handle_requests();
}

LeaderNode::~LeaderNode() {
}

void LeaderNode::handle_coord_query() {
    printf("===== COORD QUERY =====\n");
    printf("LeaderNode %d\n", local_rank);
    print_msg_info();
}

void LeaderNode::handle_partner_query() {
    printf("===== PARTNER QUERY =====\n");
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

            switch (msg_info.tag) {
                case COORD_QUERY:
                    handle_coord_query();
                    break;

                case PARTNER_QUERY:
                    handle_partner_query();
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

    // TODO: Expand this to poll all sleds that this swing node is servicing!
    // Check to see if any other cache nodes are attempting to talk to you.
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

    if (flag == 1) {
        msg_info.tag = status.MPI_TAG; 
        msg_info.src = status.MPI_SOURCE;
        msg_info.comm = MPI_COMM_WORLD;
        MPI_Get_count(&status, MPI_BYTE, &msg_info.count);
        msg_queue.push(msg_info);
    }

    // TODO: Handle messages from JOB nodes! (maybe not if we go for cache
    // processes running as threads within the job processes).
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
    printf("tag:   %d\n", msg_info.tag);
    printf("src:   %d\n", msg_info.src);
    printf("count: %d\n", msg_info.count);

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

void LeaderNode::spawn_cache_nodes(MPI_Comm *parent, int count) {
    printf("LeaderNode sending SPAWN_CACHE of size %d\n", count);
    int swing_comm_size;
    MPI_Comm_remote_size(swing_comm, &swing_comm_size);

    // TODO: Look into MPI_Comm_Idup and perhaps MPI_Bcast for sending out this
    //       spawn request to all nodes efficiently and having them all handle it
    //       efficiently.
    for (int i = 0; i < swing_comm_size; ++i) {
        printf("Leader sending spawn cache msg to swing node %d\n", i);
        MPI_Send(&count, 1, MPI_INT, i, SPAWN_CACHE, *parent);
    }
}
