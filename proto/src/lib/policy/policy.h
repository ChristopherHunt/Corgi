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
};

#endif
