#ifndef __POLICY__H__
#define __POLICY__H__

#include <stdint.h>
#include <string>
#include <vector>

class Policy {
    protected:
        // Make the destructor virtual so that there isn't undefined behavior
        // if someone tries to "delete" a Policy reference.
        virtual ~Policy() {};

    public:
        // Adds the tuple described by the key/value pair to the cache, storing
        // the copy on the local cache node as well as updating it within the
        // cache as a whole.
        virtual void put(const std::string& key, const std::string& value) = 0;

        // Gets the most up-to-date copy of the data corresponding to the input
        // key, returning the result by overwritting the value input.
        virtual void get(const std::string& key, std::string& value) = 0;

        // Pushes a key/value tuple to another node within the cache (the local
        // copy is still retained on the initial node).
        virtual int32_t push(const std::string& key, uint32_t node_id) = 0;

        // Drops the key/value tuple associated with the input key from the
        // calling node.
        virtual int32_t drop(const std::string& key) = 0;

        // Coalesces all entries for the specified key to the calling node, with
        // the final value being the most current value associated with the key
        // from any of the nodes in the cache.
        virtual int32_t collect(const std::string& key) = 0;

        // Populates the owners vector with a list of all nodes within the
        // system that have the tuple specified by the input key.
        virtual void get_owners(const std::string& key,
            std::vector<uint32_t>& owners) = 0;
};

#endif
