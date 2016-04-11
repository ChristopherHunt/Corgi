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

   public:
      CacheQuorum(Node *node);

      virtual ~CacheQuorum();

      // Method to handle a PUT message received by either a cache or swing
      // node.
      virtual void handle_put();

      // Method to handle a PUT_ACK message received by either a cache or
      // swing node.
      virtual void handle_put_ack();


      virtual void handle_put_local();
      virtual void handle_put_local_ack();

      // Method to handle a GET message received by either a cache or swing
      // node.
      virtual void handle_get();

      // Method to handle a GET_ACK message received by either a cache or
      // swing node.
      virtual void handle_get_ack();

      virtual void handle_get_local();
      virtual void handle_get_local_ack();

      virtual void handle_push();
      virtual void handle_push_ack();

      virtual void handle_pull();
      virtual void handle_pull_ack();

      virtual void handle_scatter();
      virtual void handle_scatter_ack();

      virtual void handle_gather();
      virtual void handle_gather_ack();

      virtual void handle_drop();
      virtual void handle_drop_ack();

      virtual void handle_collect();
      virtual void handle_collect_ack();

      virtual void handle_get_owners();
      virtual void handle_get_owners_ack();
};

#endif
