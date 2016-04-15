#ifndef __CACHE__QUORUM__H__
#define __CACHE__QUORUM__H__

#include <stdint.h>
#include <unordered_map>
#include <string>
#include "policy/quorum/quorum.h"

class CacheQuorum : public virtual Quorum {
   private:
      // Sends the most up to date value corresponding to a quorum vote on
      // keys and timestamps to the job node which requested it.
      void send_job_node_get_ack(std::vector<GetReq>::iterator get_req_it);

      // Parses the final stage of a PUSH_LOCAL message to the cache node.
      // buf contains a put message which is terminating at this cache node.
      void parse_and_save_push_local(uint8_t *buf, MsgInfo *msg_info);

      // Lookup a key within this cache node and push it to the next node.
      // buf contains a PUSH_LOCAL message which specifies a key/value pair to
      // lookup and send to a specified node.
      void push_to_target_node(uint8_t *buf, MsgInfo *msg_info);

   public:
      CacheQuorum(Node *node);

      virtual ~CacheQuorum();

      virtual void handle_put();
      virtual void handle_put_ack();

      // Method to handle a PUT_LOCAL message received from a job node. This
      // places a key/value pair in the cache node's cache, and informs the
      // cache node's coordinator swing node of the key if it is new to the
      // cache node.
      virtual void handle_put_local();

      // Method to handle a PUT_LOCAL_ACK message received from a swing node in
      // the event that the cache node did not have the initial key (and as a
      // result contacted the swing node). Upon hearing from the swing node, the
      // cache node then replies back to its job node that the put is done.
      virtual void handle_put_local_ack();

      // TODO: THESE ARE PARTIALLY IMPLEMENTED AND REQUIRE MORE CONSISTENCY.
      virtual void handle_get();
      virtual void handle_get_ack();

      // Method to handle a GET_LOCAL message received from a job node. Replies
      // with a message containing only the local copy of the key/value pair.
      virtual void handle_get_local();

      // Not needed in this implementation.
      virtual void handle_get_local_ack();

      virtual void handle_push();
      virtual void handle_push_ack();

      // TODO
      virtual void handle_push_local();
      virtual void handle_push_local_ack();

      virtual void handle_pull();
      virtual void handle_pull_ack();

      // TODO
      virtual void handle_pull_local();
      virtual void handle_pull_local_ack();

      virtual void handle_scatter();
      virtual void handle_scatter_ack();

      virtual void handle_scatter_local();
      virtual void handle_scatter_local_ack();

      virtual void handle_gather();
      virtual void handle_gather_ack();

      virtual void handle_drop();
      virtual void handle_drop_ack();

      // TODO
      virtual void handle_drop_local();
      virtual void handle_drop_local_ack();

      virtual void handle_collect();
      virtual void handle_collect_ack();

      virtual void handle_get_owners();
      virtual void handle_get_owners_ack();
};

#endif
