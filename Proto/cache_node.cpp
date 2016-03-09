#include "cache_node.h"

CacheNode::CacheNode() {
    // Determine where this node is in the system.
    orient();

    // Setup the socket to communicate with job processes.
    setup_socket();

    // Handle all requests sent to this cache node.
    handle_requests();
}

CacheNode::~CacheNode() {
}

void CacheNode::handle_connect() {
    printf("===== CONNECT =====\n");
    printf("CacheNode %d\n", local_rank);
    print_msg_info();
}

void CacheNode::handle_put() {
    printf("===== PUT =====\n");
    printf("CacheNode %d\n", local_rank);
    print_msg_info();
}

void CacheNode::handle_put_ack() {
    printf("===== PUT_ACK =====\n");
    printf("CacheNode %d\n", local_rank);
    print_msg_info();
}

void CacheNode::handle_get() {
    printf("===== GET =====\n");
    printf("CacheNode %d\n", local_rank);
    print_msg_info();
}

void CacheNode::handle_forward() {
    printf("===== FORWARD =====\n");
    printf("CacheNode %d\n", local_rank);
    print_msg_info();
}

void CacheNode::handle_delete() {
    printf("===== DELETE =====\n");
    printf("CacheNode %d\n", local_rank);
    print_msg_info();
}

void CacheNode::handle_delete_ack() {
    printf("===== DELETE_ACK =====\n");
    printf("CacheNode %d\n", local_rank);
    print_msg_info();
}

void CacheNode::handle_exit() {
    printf("===== EXIT =====\n");
    printf("CacheNode %d\n", local_rank);
    print_msg_info();
}

void CacheNode::handle_requests() {
    while (true) {
        while (msg_ready() == false) {
            message_select();
        }

        while (msg_ready() == true) {
            msg_info = msg_queue.front();
            msg_queue.pop();

            switch (msg_info.tag) {
                case CONNECT:
                    printf("===== CONNECT =====\n");
                    printf("CacheNode %d\n", local_rank);
                    print_msg_info();
                    break;

                case PUT:
                    printf("===== PUT =====\n");
                    printf("CacheNode %d\n", local_rank);
                    print_msg_info();
                    break;

                case PUT_ACK:
                    printf("===== PUT_ACK =====\n");
                    printf("CacheNode %d\n", local_rank);
                    print_msg_info();
                    break;

                case GET:
                    printf("===== GET =====\n");
                    printf("CacheNode %d\n", local_rank);
                    print_msg_info();
                    break;

                case GET_ACK:
                    printf("===== GET_ACK =====\n");
                    printf("CacheNode %d\n", local_rank);
                    print_msg_info();
                    break;

                case FORWARD:
                    printf("===== FORWARD =====\n");
                    printf("CacheNode %d\n", local_rank);
                    print_msg_info();
                    break;

                case DELETE:
                    printf("===== DELETE =====\n");
                    printf("CacheNode %d\n", local_rank);
                    print_msg_info();
                    break;

                case DELETE_ACK:
                    printf("===== DELETE_ACK =====\n");
                    printf("CacheNode %d\n", local_rank);
                    print_msg_info();
                    break;

                case EXIT:
                    printf("===== EXIT =====\n");
                    printf("CacheNode %d\n", local_rank);
                    print_msg_info();
                    break;

                default:
                    printf("===== DEFAULT =====\n");
                    printf("CacheNode %d\n", local_rank);
                    print_msg_info();
                    ASSERT_TRUE(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
                    break;
            }
        }
    }
}

void CacheNode::message_select() {
    int flag;

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

    // TODO: Handle messages from JOB node! (maybe not if we go and make cache
    // nodes a thread process running inside the job processes).
}

bool CacheNode::msg_ready() {
    return msg_queue.size() > 0 ? true : false;
}

void CacheNode::orient() {
    // Get data on local comm
    MPI_Comm_size(MPI_COMM_WORLD, &local_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &local_rank);

    // Get parent comm
    MPI_Comm_get_parent(&parent_comm);

    if (parent_comm == MPI_COMM_NULL) {
        printf("CacheNode %d could not access parent comm.\n", local_rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Get data on parent comm
    MPI_Comm_remote_size(parent_comm, &parent_size);
    MPI_Comm_rank(parent_comm, &parent_rank);

    // Ask parent rank 0 process asking for coordinator swing node's rank.
    MPI_Send(&local_rank, 1, MPI_INT, 0, 0, parent_comm);

    // Wait for response from parent.
    MPI_Recv(&coord_rank, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, parent_comm,
        &status);
}

void CacheNode::print_msg_info() {
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

void CacheNode::setup_socket() {
}
