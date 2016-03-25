#include "cache_node.h"

// TODO: NEED TO ADD FUNCTIONALITY TO ADD JOBS WITHOUT ADDING CACHE NODES.

CacheNode::CacheNode(std::vector<uint32_t>& mapping) {
    // Determine where this node is in the system.
    orient();

    // Get coordinator swing node's rank
    coord_rank = mapping[local_rank];

    printf("CacheNode %d's coord swing node is %d\n", local_rank, coord_rank);

    // Allocates data structures within this object.
    allocate();

    // Setup the socket to communicate with job processes.
    //setup_socket();

    // Handle all requests sent to this cache node.
    handle_requests();
}

CacheNode::~CacheNode() {
    free(buf);
}

void CacheNode::allocate() {
    buf = (uint8_t *)calloc(INITIAL_BUF_SIZE, sizeof(uint8_t));
    ASSERT_TRUE(buf != NULL, MPI_Abort(MPI_COMM_WORLD, 1));
}

void CacheNode::handle_connect() {
    printf("===== CONNECT =====\n");
    printf("CacheNode %d\n", local_rank);
    print_msg_info(&msg_info);
}

void CacheNode::handle_put() {
    printf("===== PUT =====\n");
    printf("CacheNode %d\n", local_rank);
    print_msg_info(&msg_info);
}

void CacheNode::handle_put_ack() {
    printf("===== PUT_ACK =====\n");
    printf("CacheNode %d\n", local_rank);
    print_msg_info(&msg_info);
}

void CacheNode::handle_get() {
    printf("===== GET =====\n");
    printf("CacheNode %d\n", local_rank);
    print_msg_info(&msg_info);
}

void CacheNode::handle_get_ack() {
    printf("===== GET_ACK =====\n");
    printf("CacheNode %d\n", local_rank);
    print_msg_info(&msg_info);
}

void CacheNode::handle_forward() {
    printf("===== FORWARD =====\n");
    printf("CacheNode %d\n", local_rank);
    print_msg_info(&msg_info);
}

void CacheNode::handle_delete() {
    printf("===== DELETE =====\n");
    printf("CacheNode %d\n", local_rank);
    print_msg_info(&msg_info);
}

void CacheNode::handle_delete_ack() {
    printf("===== DELETE_ACK =====\n");
    printf("CacheNode %d\n", local_rank);
    print_msg_info(&msg_info);
}

void CacheNode::handle_exit() {
    printf("===== EXIT =====\n");
    printf("CacheNode %d\n", local_rank);
    print_msg_info(&msg_info);
}

void CacheNode::handle_requests() {
    printf("CacheNode %d entering handle_requests!\n", local_rank);
    while (true) {
        while (msg_ready() == false) {
            message_select();
        }

        while (msg_ready() == true) {
            msg_info = msg_queue.front();
            msg_queue.pop();

            switch (msg_info.tag) {
                case CONNECT:
                    handle_connect();
                    break;

                case PUT:
                    handle_put();
                    break;

                case PUT_ACK:
                    handle_put_ack();
                    break;

                case GET:
                    handle_get();
                    break;

                case GET_ACK:
                    handle_get_ack();
                    break;

                case FORWARD:
                    handle_forward();
                    break;

                case DELETE:
                    handle_delete();
                    break;

                case DELETE_ACK:
                    handle_delete_ack();
                    break;

                case SPAWN_JOB:
                    handle_spawn_job();
                    break;

                case EXIT:
                    handle_exit();
                    break;

                default:
                    printf("===== DEFAULT =====\n");
                    printf("CacheNode %d\n", local_rank);
                    print_msg_info(&msg_info);
                    ASSERT_TRUE(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
                    break;
            }
        }
    }
}

void CacheNode::handle_spawn_job() {
    printf("===== SPAWN JOB =====\n");
    printf("CacheNode %d\n", local_rank);
    print_msg_info(&msg_info);

    MPI_Recv(buf, msg_info.count, MPI_UINT8_T, msg_info.src, SPAWN_JOB,
            msg_info.comm, &status);

    ASSERT_TRUE(msg_info.count == sizeof(SpawnNodesTemplate),
            MPI_Abort(MPI_COMM_WORLD, 1));

    SpawnNodesTemplate *format = (SpawnNodesTemplate *)buf;
    uint32_t job_num = format->job_num;
    uint16_t node_count = format->count;
    uint16_t mapping_size = format->mapping_size;
    printf("CacheNode %d msg_info.count: %u\n", local_rank, msg_info.count);
    printf("CacheNode %d mapping_size: %d\n", local_rank, mapping_size);
    std::string mapping_str(format->mapping, format->mapping + mapping_size);
    std::vector<char> mapping;
    stringlist_to_vector(mapping, mapping_str);

    uint8_t exec_size = format->exec_size;
    std::string exec_name(format->exec_name, format->exec_name + exec_size);

    printf("CacheNode %d received msg\n", local_rank);
    printf("CacheNode %d job #: %d\n", local_rank, job_num);
    printf("CacheNode %d spawn count: %d\n", local_rank, node_count);
    printf("CacheNode %d mapping: %s\n", local_rank, mapping_str.c_str());
    printf("CacheNode %d mapping as a vector:\n", local_rank);
    for (std::vector<char>::iterator it = mapping.begin();
            it != mapping.end(); ++it) {
        std::cout << *it << std::endl;
    }
    std::cout << std::endl;
    printf("CacheNode %d exec_size: %d\n", local_rank, exec_size);
    printf("CacheNode %d exec_name: %s\n", local_rank, exec_name.c_str());

    // Create an array that maps each job node to its corresponding cache node.
    char *argv[2];
    argv[0] = &mapping[0];
    argv[1] = NULL;

    MPI_Comm comm;
    MPI_Comm_dup(MPI_COMM_WORLD, &comm);

    // Spawn the job nodes with the appropriate argv mapping.
    MPI_Comm_spawn(exec_name.c_str(), argv, node_count, MPI_INFO_NULL,
        0, comm, &comm, MPI_ERRCODES_IGNORE);

    // Update bookkeeping.
    CommGroup job_comm_group;
    job_comm_group.swing = msg_info.comm;
    job_comm_group.cache = MPI_COMM_WORLD;
    job_comm_group.job = comm;

    job_to_comms[job_num] = job_comm_group;
    printf("CacheNode %d finished spawning Job %d\n", local_rank, job_num);
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

    printf("-----> CacheNode %d here!\n", local_rank);

    // Get parent comm
    MPI_Comm_get_parent(&parent_comm);

    if (parent_comm == MPI_COMM_NULL) {
        printf("CacheNode %d could not access parent comm.\n", local_rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Get data on parent comm
    MPI_Comm_remote_size(parent_comm, &parent_size);
    MPI_Comm_rank(parent_comm, &parent_rank);

    printf("CacheNode %d see's parent's comm size: %d\n", local_rank, parent_size); 
}

/*
   void CacheNode::print_msg_info() {
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
   */

void CacheNode::setup_socket() {
}
