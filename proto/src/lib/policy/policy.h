#ifndef __POLICY__H__
#define __POLICY__H__

#include <stdint.h>
#include <string>
#include <vector>
#include "network/network.h"
#include "shared/node.h"

class Policy {
   public:
      // Make the destructor virtual so that there isn't undefined behavior
      // if someone tries to "delete" a Policy reference.
      virtual ~Policy() {};

      virtual void handle_put() = 0;
      virtual void handle_put_ack() = 0;

      virtual void handle_get() = 0;
      virtual void handle_get_ack() = 0;

      virtual void handle_push() = 0;
      virtual void handle_push_ack() = 0;

      virtual void handle_pull() = 0;
      virtual void handle_pull_ack() = 0;

      virtual void handle_scatter() = 0;
      virtual void handle_scatter_ack() = 0;

      virtual void handle_gather() = 0;
      virtual void handle_gather_ack() = 0;

      virtual void handle_drop() = 0;
      virtual void handle_drop_ack() = 0;

      virtual void handle_collect() = 0;
      virtual void handle_collect_ack() = 0;

      virtual void handle_put_local() = 0;
      virtual void handle_put_local_ack() = 0;

      virtual void handle_get_local() = 0;
      virtual void handle_get_local_ack() = 0;

      virtual void handle_push_local() = 0;
      virtual void handle_push_local_ack() = 0;

      virtual void handle_pull_local() = 0;
      virtual void handle_pull_local_ack() = 0;

      virtual void handle_drop_local() = 0;
      virtual void handle_drop_local_ack() = 0;

      virtual void handle_scatter_local() = 0;
      virtual void handle_scatter_local_ack() = 0;

      virtual void handle_get_owners() = 0;
      virtual void handle_get_owners_ack() = 0;
};

#endif
