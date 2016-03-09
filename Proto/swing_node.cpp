#include "swing_node.h"

SwingNode::SwingNode() {
    // Determine where this node is in the system.
    orient();

    // Handle all requests sent to this cache node.
    handle_requests();
}

SwingNode::~SwingNode() {
}

void SwingNode::handle_put() {
    printf("===== PUT =====\n");
    printf("SwingNode %d\n", local_rank);
    print_msg_info();
}

void SwingNode::handle_get() {
    printf("===== GET =====\n");
    printf("SwingNode %d\n", local_rank);
    print_msg_info();
}

void SwingNode::handle_delete() {
    printf("===== DELETE =====\n");
    printf("SwingNode %d\n", local_rank);
    print_msg_info();
}

void SwingNode::handle_forward() {
    printf("===== FORWARD =====\n");
    printf("SwingNode %d\n", local_rank);
    print_msg_info();
}

void SwingNode::handle_delete_ack() {
    printf("===== DELETE_ACK =====\n");
    printf("SwingNode %d\n", local_rank);
    print_msg_info();
}

void SwingNode::handle_coord_query() {
    printf("===== COORD QUERY =====\n");
    printf("SwingNode %d\n", local_rank);
    print_msg_info();
}

void SwingNode::handle_spawn() {
    printf("===== SPAWN =====\n");
    printf("SwingNode %d\n", local_rank);
    print_msg_info();
}

void SwingNode::handle_exit() {
    printf("===== EXIT =====\n");
    printf("SwingNode %d\n", local_rank);
    print_msg_info();
}

void SwingNode::handle_requests() {
    while (true) {
        while (msg_ready() == false) {
            message_select();
        }

        while (msg_ready() == true) {
            msg_info = msg_queue.front();
            msg_queue.pop();

            switch (msg_info.tag) {
                case PUT:
                    handle_put();
                    break;

                case GET:
                    handle_get();
                    break;

                case DELETE:
                    handle_delete();
                    break;

                case DELETE_ACK:
                    handle_delete_ack();
                    break;

                case FORWARD:
                    handle_forward();
                    break;

                case COORD_QUERY:
                    handle_coord_query();
                    break;

                case SPAWN:
                    handle_spawn();
                    break;

                case EXIT:
                    handle_exit();
                    break;

                default:
                    printf("===== DEFAULT =====\n");
                    printf("SwingNode %d\n", local_rank);
                    print_msg_info();
                    ASSERT_TRUE(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
                    break;
            }
        }
    }
}

void SwingNode::message_select() {
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

    // Check to see if any other cache nodes are attempting to talk to you.
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, parent_comm, &flag, &status);

    if (flag == 1) {
        msg_info.tag = status.MPI_TAG; 
        msg_info.src= status.MPI_SOURCE;
        msg_info.comm = parent_comm;
        MPI_Get_count(&status, MPI_BYTE, &msg_info.count);
        msg_queue.push(msg_info);
    }

    // TODO: Handle messages from JOB nodes! (maybe not if we go for cache
    // processes running as threads within the job processes).
}

bool SwingNode::msg_ready() {
    return msg_queue.size() > 0 ? true : false;
}

void SwingNode::orient() {
    // Get data on local comm
    MPI_Comm_size(MPI_COMM_WORLD, &local_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &local_rank);

    // Get parent comm
    MPI_Comm_get_parent(&parent_comm);

    if (parent_comm == MPI_COMM_NULL) {
        printf("SwingNode %d could not access parent comm.\n", local_rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Get data on parent comm
    MPI_Comm_remote_size(parent_comm, &parent_size);
    MPI_Comm_rank(parent_comm, &parent_rank);
}

void SwingNode::print_msg_info() {
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
