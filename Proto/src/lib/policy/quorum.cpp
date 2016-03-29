#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include "quorum.h"

Quorum::Quorum(Node *node, NodeType node_type) {
    printf("Quorum constructor!\n"); 
    this->node_type = node_type;
    this->node = node;
}

Quorum::~Quorum() {
    printf("Quorum destructor!\n"); 
}

void Quorum::swing_node_handle_put(uint8_t *buf, MsgInfo *msg_info) {
    printf("SWING_NODE handle_put\n");
    print_msg_info(msg_info);

    PutTemplate *format = (PutTemplate *)buf;
    std::string key(format->key, format->key + format->key_size);

    JobNodeID id;
    id.job_num = format->job_num;
    id.job_node = format->job_node;

    // If the swing node did not have a mapping of the key to any nodes, add
    // the key mapping.
    if (node->key_to_nodes.count(key) == 0) {
        printf("----- Swing node %d did not have key %s in its key_to_nodes!\n",
                node->local_rank, key.c_str());
        std::vector<JobNodeID> v;
        v.push_back(id);
        node->key_to_nodes[key] = v;
    }
    // If the swing node had a mapping of the key to some nodes, ensure that the
    // new node that has the key is included in that mapping.
    else {
        printf("+++++ Swing node %d had key %s in its key_to_nodes!\n",
                node->local_rank, key.c_str());
        std::vector<JobNodeID> v = node->key_to_nodes[key];
        if (std::find(v.begin(), v.end(), id) == v.end()) {
            node->key_to_nodes[key].push_back(id);
        }

    }

    // TODO: REMOVE
    std::vector<JobNodeID> v = node->key_to_nodes[key];
    for (std::vector<JobNodeID>::iterator it = v.begin();
            it != v.end(); ++it) {
        printf("Swing node %d sees vector for key %s containing node %d of job %d\n",
                node->local_rank, key.c_str(), it->job_node, it->job_num);
    }
    //

    // TODO:
    // CURRENTLY THIS IS JUST REPLYING IMMEDIATELY TO TEST FUNCTIONALITY,
    // INSTEAD IT NEEDS TO PROPOGRATE THIS NEW KEY INFORMATION TO ALL OTHER
    // SWING NODES SO THAT THEY ALL HAVE A CONCURRENT VIEW OF THE SYSTEM.
    MPI_Request request;
    send_msg(buf, sizeof(PutAckTemplate), MPI_UINT8_T, msg_info->src, PUT_ACK,
            msg_info->comm, &request);
}

void Quorum::cache_node_handle_put(uint8_t *buf, MsgInfo *msg_info) {
    printf("CACHE_NODE handle_put\n");

    // Parse the Put message within buf, and add it to the cache.
    PutTemplate *format = (PutTemplate *)buf;
    std::string key(format->key, format->key + format->key_size);

    Parcel parcel;
    parcel.timestamp = format->timestamp;
    parcel.value = std::string(format->value, format->value + format->value_size);
    cache[key] = parcel;

    // Send msg to parent swing node.
    MPI_Request request;
    send_msg(buf, msg_info->count, MPI_UINT8_T, node->coord_rank, PUT,
            node->parent_comm, &request);
}

void Quorum::swing_node_handle_put_ack(uint8_t *buf, MsgInfo *msg_info) {
    printf("SWING_NODE handle_put_ack\n");
}

void Quorum::cache_node_handle_put_ack(uint8_t *buf, MsgInfo *msg_info) {
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

void Quorum::swing_node_handle_get(uint8_t *buf, MsgInfo *msg_info) {
    printf("SWING_NODE handle_get\n");

    GetTemplate *format = (GetTemplate *)buf;
    uint32_t job_num = format->job_num;
    uint32_t job_node = format->job_node;
    uint32_t key_size = format->key_size;
    std::string key(format->key, format->key + key_size);
    uint64_t timestamp = format->timestamp;

    // Grab the list of nodes which have a value associated with the target key.
    std::vector<JobNodeID> nodes = node->key_to_nodes[key];

    // Build the new message to send to the calling cache node (a lot of this is
    // reassignment but its safer for now to do it this way, we can strip out
    // the reassihnmnet to the buffer later).
    CensusTemplate *census_format = (CensusTemplate *)buf;
    census_format->job_num = job_num;
    census_format->job_node = job_node;
    census_format->key_size = key_size;
    memcpy(census_format->key, key.c_str(), key_size);

    if (nodes.size() == 0) {
        census_format->votes_req = 0;
    }
    else {
        census_format->votes_req = nodes.size();
    }

    // Send the updated votes required amount to the cache_node.
    MPI_Request request;
    send_msg(buf, sizeof(CensusTemplate), MPI_UINT8_T, msg_info->src, GET_ACK,
            msg_info->comm, &request);

    // TODO: NOW NEED TO HAVE THIS SWING NODE PING ALL OF THE OTHER CACHE NODES
    // SO THAT THEY CAN UPDATE THE CALLING CACHE NODE.
}

void Quorum::cache_node_handle_get(uint8_t *buf, MsgInfo *msg_info) {
    printf("CACHE_NODE handle_get\n");

    GetTemplate *format = (GetTemplate *)buf;

    GetReq get_req;
    get_req.job_num = format->job_num;
    get_req.job_node = format->job_node;
    get_req.key = std::string(format->key, format->key + format->key_size);

    // Set max voter size and upate later when the cache_node hears back from 
    // its coord_swing node.
    get_req.votes_req = node->local_size;
    get_req.census = std::vector<Parcel>();

    if (cache.count(get_req.key) != 0) {
        get_req.census.push_back(cache[get_req.key]);
    }
    pending_get_requests.push_back(get_req);

    // Send msg to parent swing node.
    MPI_Request request;
    send_msg(buf, msg_info->count, MPI_UINT8_T, node->coord_rank, GET,
            node->parent_comm, &request);
}

void Quorum::swing_node_handle_get_ack(uint8_t *buf, MsgInfo *msg_info) {
    printf("SWING_NODE handle_get_ack\n");
}

void Quorum::cache_node_handle_get_ack(uint8_t *buf, MsgInfo *msg_info) {
    printf("CACHE_NODE handle_get_ack\n");

    // Iterator to find the GetReq that matches this get_ack.
    std::vector<GetReq>::iterator it;

    // If this is a message from this cache_node's coordinator swing_node
    if (msg_info->comm == node->parent_comm && msg_info->src == node->coord_rank) {
        // Build out the match for the GetReq entry from the message.
        CensusTemplate *format = (CensusTemplate *)buf;
        GetReq updated_vote;
        updated_vote.job_num = format->job_num;
        updated_vote.job_node = format->job_node;
        updated_vote.key =
            std::string(format->key, format->key + format->key_size);

        // Match the GetReq object to an entry in pending_get_requests.
        it = std::find(pending_get_requests.begin(), pending_get_requests.end(),
                updated_vote);

        if (it == pending_get_requests.end()) {
            // This should never happen so abort.
            ASSERT_TRUE(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
        }
        else {
            // Update the votes_required
            it->votes_req = format->votes_req;
        }
    }
    else {
        GetAckTemplate *format = (GetAckTemplate *)buf;
        GetReq new_vote;
        new_vote.job_num = format->job_num;
        new_vote.job_node = format->job_node;
        new_vote.key =
            std::string(format->key, format->key + format->key_size);

        // Match the GetReq object to an entry in pending_get_requests.
        it = std::find(pending_get_requests.begin(), pending_get_requests.end(),
                new_vote);

        if (it == pending_get_requests.end()) {
            // This should never happen so abort.
            ASSERT_TRUE(1 == 0, MPI_Abort(MPI_COMM_WORLD, 1));
        }
        else {
            // Add the new value to the vector of votes.
            Parcel vote;
            vote.value = std::string (format->value,
                format->value + format->value_size);
            vote.timestamp = format->timestamp;
            it->census.push_back(vote);
        }
    }

    if (it->votes_req == it->census.size()) {
        send_job_node_get_ack(it);
        pending_get_requests.erase(it);
    }
    else {
        printf("CacheNode %d - key %s - votes %d/%d\n", node->local_rank,
            it->key.c_str(), it->census.size(), it->votes_req);
    }
}

void Quorum::send_job_node_get_ack(std::vector<GetReq>::iterator get_req_it) {
    printf("SEND_JOB_NODE_GET_ACK!\n");
    std::vector<Parcel> census = get_req_it->census;
    Parcel winning_vote;
    winning_vote.timestamp = 0;

    for (std::vector<Parcel>::iterator it = census.begin();
        it != census.end(); ++it) {
        if (it->timestamp > winning_vote.timestamp) {
            winning_vote.timestamp = it->timestamp;
            winning_vote.value = it->value;
        }
    }

    GetAckTemplate *format = (GetAckTemplate *)node->buf;
    format->job_num = get_req_it->job_num;
    format->job_node = get_req_it->job_node;
    format->key_size = get_req_it->key.size();
    memcpy(format->key, get_req_it->key.c_str(), format->key_size);
    format->value_size = winning_vote.value.size();
    memcpy(format->value, winning_vote.value.c_str(), winning_vote.value.size());
    format->timestamp = winning_vote.timestamp;

    MPI_Comm job_comm = node->job_to_comms[format->job_num].job;
    MPI_Request request;
    
    send_msg(node->buf, sizeof(GetAckTemplate), MPI_UINT8_T, format->job_node,
        GET_ACK, job_comm, &request);
}

void Quorum::handle_put() {
    uint8_t *buf = node->buf;
    MsgInfo *msg_info = &(node->msg_info);
    MPI_Status *status = &(node->status);

    recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, PUT,
            msg_info->comm, status);

    switch (node_type) {
        case SWING:
            swing_node_handle_put(buf, msg_info);
            break;

        case CACHE:
            cache_node_handle_put(buf, msg_info);
            break;

        default:
            ASSERT_TRUE(1 == 0, MPI_Abort(1, MPI_COMM_WORLD));
            break;
    }
}

void Quorum::handle_put_ack() {
    uint8_t *buf = node->buf;
    MsgInfo *msg_info = &(node->msg_info);
    MPI_Status *status = &(node->status);

    recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, PUT_ACK,
            msg_info->comm, status);

    switch (node_type) {
        case SWING:
            swing_node_handle_put_ack(buf, msg_info);
            break;

        case CACHE:
            cache_node_handle_put_ack(buf, msg_info);
            break;

        default:
            ASSERT_TRUE(1 == 0, MPI_Abort(1, MPI_COMM_WORLD));
            break;
    }
}

void Quorum::handle_get() {
    uint8_t *buf = node->buf;
    MsgInfo *msg_info = &(node->msg_info);
    MPI_Status *status = &(node->status);

    recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, GET,
            msg_info->comm, status);

    switch (node_type) {
        case SWING:
            swing_node_handle_get(buf, msg_info);
            break;

        case CACHE:
            cache_node_handle_get(buf, msg_info);
            break;

        default:
            ASSERT_TRUE(1 == 0, MPI_Abort(1, MPI_COMM_WORLD));
            break;
    }
}

void Quorum::handle_get_ack() {
    uint8_t *buf = node->buf;
    MsgInfo *msg_info = &(node->msg_info);
    MPI_Status *status = &(node->status);

    recv_msg(buf, msg_info->count, MPI_UINT8_T, msg_info->src, GET_ACK,
            msg_info->comm, status);

    switch (node_type) {
        case SWING:
            swing_node_handle_get_ack(buf, msg_info);
            break;

        case CACHE:
            cache_node_handle_get_ack(buf, msg_info);
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
