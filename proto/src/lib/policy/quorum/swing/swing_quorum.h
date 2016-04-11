#ifndef __SWING__QUORUM__H__
#define __SWING__QUORUM__H__

#include <stdint.h>
#include <unordered_map>
#include <string>
#include "policy/quorum/quorum.h"

class SwingQuorum : public virtual Quorum {
   public:
      SwingQuorum(Node *node);

      virtual ~SwingQuorum();

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
