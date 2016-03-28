#include "quorum.h"
#include <stdio.h>

Quorum::Quorum() {
    printf("Quorum constructor!\n"); 
}

Quorum::~Quorum() {
    printf("Quorum destructor!\n"); 
}

void Quorum::put(const std::string& key, const std::string& value) {

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
