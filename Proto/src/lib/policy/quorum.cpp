#include <stdio.h>
#include <mpi.h>
#include "quorum.h"

Quorum::Quorum(NodeType node_type) {
    printf("Quorum constructor!\n"); 
    this->node_type = node_type;
}

Quorum::~Quorum() {
    printf("Quorum destructor!\n"); 
}

void Quorum::swing_node_handle_put(Node *node, uint8_t *buf, MsgInfo *msg_info) {
    printf("SWING_NODE handle_put\n");
    print_msg_info(msg_info);

    // TODO: FLESH THIS OUT
    // CURRENTLY THIS IS JUST REPLYING IMMEDIATELY TO TEST FUNCTIONALITY
    MPI_Request request;
    send_msg(buf, sizeof(PutAckTemplate), MPI_UINT8_T, msg_info->src, PUT_ACK,
        msg_info->comm, &request);
}

void Quorum::cache_node_handle_put(Node *node, uint8_t *buf, MsgInfo *msg_info) {
    printf("CACHE_NODE handle_put\n");

    // Parse the Put message within buf, and add it to the cache.
    PutTemplate *format = (PutTemplate *)buf;
    std::string key(format->key, format->key + format->key_size);
    std::string value(format->value, format->value + format->value_size);
    cache[key] = value;

    // Send msg to parent swing node.
    MPI_Request request;
    send_msg(buf, msg_info->count, MPI_UINT8_T, node->coord_rank, PUT,
        node->parent_comm, &request);
}

void Quorum::swing_node_handle_put_ack(Node *node, uint8_t *buf, MsgInfo *msg_info) {
    printf("SWING_NODE handle_put_ack\n");
}

void Quorum::cache_node_handle_put_ack(Node *node, uint8_t *buf, MsgInfo *msg_info) {
    printf("CACHE_NODE handle_put_ack\n");

    // Parse the Put message within buf, and add it to the cache.
    PutAckTemplate *format = (PutAckTemplate *)buf;
    uint32_t job_num = format->job_num;
    uint32_t job_node = format->job_node;

    MPI_Comm job_comm = node->job_to_comms[job_num].job;

    // Send msg to job node informing it that the put is complete.
    MPI_Request request;
    send_msg(buf, sizeof(PutAckTemplate), MPI_UINT8_T, job_node, PUT_ACK, job_comm,
        &request);
}

void Quorum::swing_node_handle_get(Node *node, uint8_t *buf, MsgInfo *msg_info) {
    printf("SWING_NODE handle_get\n");
}

void Quorum::cache_node_handle_get(Node *node, uint8_t *buf, MsgInfo *msg_info) {
    printf("CACHE_NODE handle_get\n");
}

void Quorum::swing_node_handle_get_ack(Node *node, uint8_t *buf, MsgInfo *msg_info) {
    printf("SWING_NODE handle_get_ack\n");
}

void Quorum::cache_node_handle_get_ack(Node *node, uint8_t *buf, MsgInfo *msg_info) {
    printf("CACHE_NODE handle_get_ack\n");
}

void Quorum::handle_put(Node *node) {
    uint8_t *buf = node->buf;
    MsgInfo *msg_info = &(node->msg_info);
    MPI_Status *status = &(node->status);

    recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, PUT,
        msg_info->comm, status);

    switch (node_type) {
        case SWING:
            swing_node_handle_put(node, buf, msg_info);
            break;

        case CACHE:
            cache_node_handle_put(node, buf, msg_info);
            break;

        default:
            ASSERT_TRUE(1 == 0, MPI_Abort(1, MPI_COMM_WORLD));
            break;
    }
}

void Quorum::handle_put_ack(Node *node) {
    uint8_t *buf = node->buf;
    MsgInfo *msg_info = &(node->msg_info);
    MPI_Status *status = &(node->status);

    recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, PUT_ACK,
        msg_info->comm, status);

    switch (node_type) {
        case SWING:
            swing_node_handle_put_ack(node, buf, msg_info);
            break;

        case CACHE:
            cache_node_handle_put_ack(node, buf, msg_info);
            break;

        default:
            ASSERT_TRUE(1 == 0, MPI_Abort(1, MPI_COMM_WORLD));
            break;
    }
}

void Quorum::handle_get(Node *node) {
    uint8_t *buf = node->buf;
    MsgInfo *msg_info = &(node->msg_info);
    MPI_Status *status = &(node->status);

    recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, GET,
        msg_info->comm, status);

    switch (node_type) {
        case SWING:
            swing_node_handle_get(node, buf, msg_info);
            break;

        case CACHE:
            cache_node_handle_get(node, buf, msg_info);
            break;

        default:
            ASSERT_TRUE(1 == 0, MPI_Abort(1, MPI_COMM_WORLD));
            break;
    }
}

void Quorum::handle_get_ack(Node *node) {
    uint8_t *buf = node->buf;
    MsgInfo *msg_info = &(node->msg_info);
    MPI_Status *status = &(node->status);

    recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, GET_ACK,
        msg_info->comm, status);

    switch (node_type) {
        case SWING:
            swing_node_handle_get_ack(node, buf, msg_info);
            break;

        case CACHE:
            cache_node_handle_get_ack(node, buf, msg_info);
            break;

        default:
            ASSERT_TRUE(1 == 0, MPI_Abort(1, MPI_COMM_WORLD));
            break;
    }
}

/*
void Quorum::handle_put(const std::string& key, const std::string& value) {
    
}

void Quorum::get(const std::string& key, std::string& value) {

}

int32_t Quorum::push(const std::string& key, uint32_t node_id) {
    return 0;
}

int32_t Quorum::drop(const std::string& key) {
    return 0;
}

int32_t Quorum::collect(const std::string& key) {
    return 0;
}

void Quorum::get_owners(const std::string& key, std::vector<uint32_t>& owners) {

}
*/
