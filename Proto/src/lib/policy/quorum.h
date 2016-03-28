#ifndef __QUORUM__H__
#define __QUORUM__H__

#include <unordered_map>
#include <string>
#include "policy.h"
#include "../node.h"

enum NodeType { SWING, CACHE };

class Quorum : public virtual Policy {
    private:
        NodeType node_type;
        std::unordered_map<std::string, std::string> cache; 

    public:
        Quorum(NodeType node_type);

        virtual ~Quorum();

        virtual void handle_put(Node *node);
        virtual void handle_put_ack(Node *node);
        virtual void handle_get(Node *node);
        virtual void handle_get_ack(Node *node);

        void swing_node_handle_put(Node *node, uint8_t *buf, MsgInfo *info);
        void swing_node_handle_put_ack(Node *node, uint8_t *buf, MsgInfo *info);
        void swing_node_handle_get(Node *node, uint8_t *buf, MsgInfo *info);
        void swing_node_handle_get_ack(Node *node, uint8_t *buf, MsgInfo *info);

        void cache_node_handle_put(Node *node, uint8_t *buf, MsgInfo *info);
        void cache_node_handle_put_ack(Node *node, uint8_t *buf, MsgInfo *info);
        void cache_node_handle_get(Node *node, uint8_t *buf, MsgInfo *info);
        void cache_node_handle_get_ack(Node *node, uint8_t *buf, MsgInfo *info);

        /*
        // Adds the tuple described by the key/value pair to the cache, storing
        // the copy on the local cache node as well as updating it within the
        // cache as a whole.
        virtual void put(const std::string& key, const std::string& value);

        // Gets the most up-to-date copy of the data corresponding to the input
        // key, returning the result by overwritting the value input.
        virtual void get(const std::string& key, std::string& value);

        // Pushes a key/value tuple to another node within the cache (the local
        // copy is still retained on the initial node).
        virtual int32_t push(const std::string& key, uint32_t node_id);

        // Drops the key/value tuple associated with the input key from the
        // calling node.
        virtual int32_t drop(const std::string& key);

        // Coalesces all entries for the specified key to the calling node, with
        // the final value being the most current value associated with the key
        // from any of the nodes in the cache.
        virtual int32_t collect(const std::string& key);

        // Populates the owners vector with a list of all nodes within the
        // system that have the tuple specified by the input key.
        virtual void get_owners(const std::string& key,
                std::vector<uint32_t>& owners);
        */
};

#endif
