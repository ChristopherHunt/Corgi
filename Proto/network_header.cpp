#include <stdio.h>
#include "network_header.h"

void print_msg_info(MsgInfo *msg_info) {
    MsgTag tag = (MsgTag)msg_info->tag;

    printf("===== MsgInfo =====\n");
    printf("tag ---------> ");
    print_msg_tag_handle(tag);
    printf("\n");
    printf("src ---------> %d\n", msg_info->src);
    printf("count (bytes): %d\n", msg_info->count);

    if (msg_info->comm == MPI_COMM_WORLD) {
        printf("comm: MPI_COMM_WORLD\n");
    }
    else {
        printf("comm: OTHER\n");
    }
}

void print_msg_tag_handle(MsgTag tag) {
    switch (tag) {
        case CONNECT:
            printf("CONNECT");
            break;
        case PUT:
            printf("PUT");
            break;
        case PUT_ACK:
            printf("PUT_ACK");
            break;
        case GET:
            printf("GET");
            break;
        case GET_ACK:
            printf("GET_ACK");
            break;
        case FORWARD:
            printf("FORWARD");
            break;
        case DELETE:
            printf("DELETE");
            break;
        case DELETE_ACK:
            printf("DELETE_ACK");
            break;
        case COORD_QUERY:
            printf("COORD_QUERY");
            break;
        case COORD_QUERY_ACK:
            printf("COORD_QUERY_ACK");
            break;
        case TEAM_QUERY:
            printf("TEAM_QUERY");
            break;
        case SPAWN_JOB:
            printf("SPAWN_JOB");
            break;
        case SPAWN_CACHE:
            printf("SPAWN_CACHE");
            break;
        case EXIT:
            printf("EXIT");
            break;
        default:
            printf("UNKNOWN FLAG!");
            break;
    }
}

void vector_to_stringlistG(std::vector<char> &vec, std::string &result) {
    result.clear();
    std::stringstream ss;
    std::copy(vec.begin(), vec.end(), std::ostream_iterator<char>(ss, ","));
    result.assign(ss.str());
}

void stringlist_to_vector(std::vector<char> &vec, std::string &result) {
    vec.clear();
    std::stringstream ss(result);
    std::copy(std::istream_iterator<char>(ss),
            std::istream_iterator<char>(), std::back_inserter(vec));
}

void vector_to_stringlist(std::vector<uint32_t> &vec, std::string &result) {
    result.clear();
    std::stringstream ss;
    std::copy(vec.begin(), vec.end(), std::ostream_iterator<uint32_t>(ss, ","));
    result.assign(ss.str());
}

void stringlist_to_vector(std::vector<uint32_t> &vec, std::string &result) {
    vec.clear();
    std::stringstream ss(result);
    std::copy(std::istream_iterator<uint32_t>(ss),
            std::istream_iterator<uint32_t>(), std::back_inserter(vec));
}

void replace_commas(std::string &str) {
    std::replace(str.begin(), str.end(), ',', ' ');
}

void remove_commas(std::vector<char> &vec) {
    vec.erase(std::remove(vec.begin(), vec.end(), ','), vec.end());
}
