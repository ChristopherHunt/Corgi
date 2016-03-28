#include <stdlib.h>
#include "swing_node.h"

SwingNode::SwingNode() {
    // Set the policy for consistency and latency.
    NodeType node_type = SWING; 
    policy = new Quorum(node_type);

    // Determine where this node is in the system.
    orient();

    // Defines MPI datatypes within this object.
    //define_datatypes();

    // Allocates data structures within this object.
    allocate();

    // Handle all requests sent to this cache node.
    handle_requests();
}

SwingNode::~SwingNode() {
    free(buf);
    delete(policy);
}

void SwingNode::allocate() {
   buf = (uint8_t *)calloc(INITIAL_BUF_SIZE, sizeof(uint8_t));
   ASSERT_TRUE(buf != NULL, MPI_Abort(MPI_COMM_WORLD, 1));
}

void SwingNode::handle_put() {
    printf("===== PUT =====\n");
    printf("SwingNode %d\n", local_rank);
    print_msg_info(&msg_info);

    policy->handle_put(this);
}

void SwingNode::handle_put_ack() {
    printf("===== PUT ACK =====\n");
    printf("SwingNode %d\n", local_rank);
    print_msg_info(&msg_info);

    policy->handle_put_ack(this);
}

void SwingNode::handle_get() {
    printf("===== GET =====\n");
    printf("SwingNode %d\n", local_rank);
    print_msg_info(&msg_info);

    policy->handle_get(this);
}

void SwingNode::handle_get_ack() {
    printf("===== GET ACK =====\n");
    printf("SwingNode %d\n", local_rank);
    print_msg_info(&msg_info);

    policy->handle_get_ack(this);
}

void SwingNode::handle_delete() {
    printf("===== DELETE =====\n");
    printf("SwingNode %d\n", local_rank);
    print_msg_info(&msg_info);
}

void SwingNode::handle_forward() {
    printf("===== FORWARD =====\n");
    printf("SwingNode %d\n", local_rank);
    print_msg_info(&msg_info);
}

void SwingNode::handle_delete_ack() {
    printf("===== DELETE_ACK =====\n");
    printf("SwingNode %d\n", local_rank);
    print_msg_info(&msg_info);
}

void SwingNode::handle_team_query() {
    printf("===== TEAM QUERY =====\n");
    printf("SwingNode %d\n", local_rank);
    print_msg_info(&msg_info);
}

void SwingNode::handle_spawn_job() {
    printf("===== SPAWN JOB =====\n");
    printf("SwingNode %d\n", local_rank);
    print_msg_info(&msg_info);

    // Receive the job spawn request from the leader.
    recv_msg(buf, msg_info.count, MPI_UINT8_T, msg_info.src, SPAWN_JOB,
            msg_info.comm, &status);

    // Ensure that the message is the correct size.
    ASSERT_TRUE(msg_info.count == sizeof(SpawnNodesTemplate),
        MPI_Abort(MPI_COMM_WORLD, 1));

    printf("SwingNode %d received spawn_job msg successfully!\n", local_rank);

    // Pull out the job number so we can know which cache nodes to send to.
    SpawnNodesTemplate *format = (SpawnNodesTemplate *)buf;
    uint32_t job_num = format->job_num;

    // There must already have been a CommsGroup entry in the job_to_comms
    // map for this job number (either from cache node creation or from cache
    // node splitting / regrouping in a previous step).
    ASSERT_TRUE(job_to_comm.count(job_num) != 0, MPI_Abort(MPI_COMM_WORLD, 1));
    MPI_Comm cache_comm = job_to_comm[job_num];

    int comm_size;
    MPI_Comm_remote_size(cache_comm, &comm_size);
    MPI_Request request;

    // Send each cache node a request to spawn a job node.
    for (int i = 0; i < comm_size; ++i) {
        printf("SwingNode %d sending spawn cache msg to cache node %d\n",
            local_rank, i);
        send_msg(buf, msg_info.count, MPI_UINT8_T, i, SPAWN_JOB, cache_comm,
            &request);
    }
}

void SwingNode::handle_spawn_cache() {
    printf("===== SPAWN CACHE =====\n");
    printf("SwingNode %d\n", local_rank);
    print_msg_info(&msg_info);

    recv_msg(buf, msg_info.count, MPI_UINT8_T, msg_info.src, SPAWN_CACHE,
            msg_info.comm, &status);

    SpawnNodesTemplate *format = (SpawnNodesTemplate *)buf;
    uint32_t job_num = format->job_num;
    uint16_t node_count = format->count;
    uint16_t mapping_size = format->mapping_size;
    printf("SwingNode %d msg_info.count: %u\n", local_rank, msg_info.count);
    printf("SwingNode %d mapping_size: %d\n", local_rank, mapping_size);
    std::string mapping_str(format->mapping, format->mapping + mapping_size);
    std::vector<char> mapping;
    stringlist_to_vector(mapping, mapping_str);

    printf("SwingNode %d received msg\n", local_rank);
    printf("SwingNode %d job #: %d\n", local_rank, job_num);
    printf("SwingNode %d spawn count: %d\n", local_rank, node_count);
    printf("SwingNode %d mapping: %s\n", local_rank, mapping_str.c_str());
    printf("SwingNode %d mapping as a vector:\n", local_rank);
    for (std::vector<char>::iterator it = mapping.begin();
        it != mapping.end(); ++it) {
        std::cout << *it << std::endl;
    }
    std::cout << std::endl;

    MPI_Comm temp;

    MPI_Comm_dup(MPI_COMM_WORLD, &temp);

    printf("SwingNode %d duplicated MPI_COMM_WORLD\n", local_rank);

    // Create an array that maps each cache node to its corresponding
    // coordinator swing node, and pass that to the nodes upon spawning.
    char *argv[2];
    argv[0] = &mapping[0];
    argv[1] = NULL;

    // All swing nodes spawn the cache nodes.
    MPI_Comm_spawn("cache_node_main", argv, node_count, MPI_INFO_NULL,
            0, temp, &temp, MPI_ERRCODES_IGNORE);

    // Ensure that this job does not have cache nodes already associated with
    // it.
    ASSERT_TRUE(job_to_comm.count(job_num) == 0, MPI_Abort(MPI_COMM_WORLD, 1));

    // Update bookkeeping.
    job_to_comm[job_num] = temp;

    printf("SwingNode %d spawned cache_nodes\n", local_rank);
}

void SwingNode::handle_exit() {
    printf("===== EXIT =====\n");
    printf("SwingNode %d\n", local_rank);
    print_msg_info(&msg_info);
}

void SwingNode::handle_requests() {
    printf("SwingNode %d entering handle_requests!\n", local_rank);
    while (true) {
        while (msg_ready() == false) {
            message_select();
        }

        while (msg_ready() == true) {
            msg_info = msg_queue.front();
            msg_queue.pop_front();

            switch (msg_info.tag) {
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

                case DELETE:
                    handle_delete();
                    break;

                case DELETE_ACK:
                    handle_delete_ack();
                    break;

                case FORWARD:
                    handle_forward();
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
                    printf("SwingNode %d\n", local_rank);
                    print_msg_info(&msg_info);
                    ASSERT_TRUE(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
                    break;
            }
        }
    }
}

void SwingNode::message_select() {
    int flag;

    // Check to see if leader is trying to talk to you.
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, parent_comm, &flag, &status);

    if (flag == 1) {
        msg_info.tag = status.MPI_TAG; 
        msg_info.src= status.MPI_SOURCE;
        msg_info.comm = parent_comm;
        MPI_Get_count(&status, MPI_BYTE, &msg_info.count);
        msg_queue.push_back(msg_info);
    }

    // See if any swing nodes are trying to talk to you.
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

    if (flag == 1) {
        msg_info.tag = status.MPI_TAG; 
        msg_info.src = status.MPI_SOURCE;
        msg_info.comm = MPI_COMM_WORLD;
        MPI_Get_count(&status, MPI_BYTE, &msg_info.count);
        msg_queue.push_back(msg_info);
    }

    for (auto const &entry : job_to_comm) {
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, entry.second, &flag, &status);

        if (flag == 1) {
            msg_info.tag = status.MPI_TAG; 
            msg_info.src = status.MPI_SOURCE;
            msg_info.comm = entry.second;
            MPI_Get_count(&status, MPI_BYTE, &msg_info.count);
            msg_queue.push_back(msg_info);
        }
    }
}

bool SwingNode::msg_ready() {
    return msg_queue.size() > 0 ? true : false;
}

void SwingNode::orient() {
    // Get data on local comm
    MPI_Comm_size(MPI_COMM_WORLD, &local_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &local_rank);

    printf("++++++> SwingNode %d here!\n", local_rank);

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

/*
void SwingNode::print_msg_info() {
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
