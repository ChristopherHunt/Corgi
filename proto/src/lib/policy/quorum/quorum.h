#ifndef __QUORUM__H__
#define __QUORUM__H__

#include <stdint.h>
#include <unordered_map>
#include <string>
#include "policy/policy.h"
#include "shared/node.h"

// Struct to encapsulate a "get" request.
typedef struct GetReq {
   uint32_t job_num;             // Number of the job the request is for
   uint32_t job_node;            // Number of the job node requesting the get
   uint32_t votes_req;           // Number of votes required for this request
   std::string key;              // Key who's value is being looked up
   std::vector<Parcel> census;   // List of votes on this request

   // Overload the equals operator so it can compare 2 GetReq structs.
   bool operator== (const GetReq& other) {
      if (this->job_num != other.job_num) {
         return false;
      }
      else if (this->job_node != other.job_node) {
         return false;
      }
      else if (this->key.compare(other.key) != 0) {
         return false;
      }
      return true;
   }
} GetReq;

class Quorum : public virtual Policy {
   public:
      Node *node;
      std::unordered_map<std::string, Parcel> cache; 
      std::vector<GetReq> pending_get_requests;

      //virtual ~Quorum() = 0;

      // Method to handle a PUT message received by either a cache or swing
      // node.
      virtual void handle_put() = 0;

      // Method to handle a PUT_ACK message received by either a cache or
      // swing node.
      virtual void handle_put_ack() = 0;


      virtual void handle_put_local() = 0;
      virtual void handle_put_local_ack() = 0;

      // Method to handle a GET message received by either a cache or swing
      // node.
      virtual void handle_get() = 0;

      // Method to handle a GET_ACK message received by either a cache or
      // swing node.
      virtual void handle_get_ack() = 0;

      virtual void handle_get_local() = 0;
      virtual void handle_get_local_ack() = 0;

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

      virtual void handle_get_owners() = 0;
      virtual void handle_get_owners_ack() = 0;
};

#endif
