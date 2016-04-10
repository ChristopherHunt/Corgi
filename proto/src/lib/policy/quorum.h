#ifndef __QUORUM__H__
#define __QUORUM__H__

#include <stdint.h>
#include <unordered_map>
#include <string>
#include "policy.h"
#include "shared/node.h"

enum NodeType { SWING, CACHE };

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
   private:
      NodeType node_type;
      Node *node;
      std::unordered_map<std::string, Parcel> cache; 
      std::vector<GetReq> pending_get_requests;

      // Sends the most up to date value corresponding to a quorum vote on
      // keys and timestamps to the job node which requested it.
      void send_job_node_get_ack(std::vector<GetReq>::iterator get_req_it);

      // Internal helper methods to handle the API cases for swing nodes.
      void swing_node_handle_put(uint8_t *buf, MsgInfo *info);
      void swing_node_handle_put_ack(uint8_t *buf, MsgInfo *info);
      void swing_node_handle_get(uint8_t *buf, MsgInfo *info);
      void swing_node_handle_get_ack(uint8_t *buf, MsgInfo *info);
      void swing_node_handle_push(uint8_t *buf, MsgInfo *info);
      void swing_node_handle_push_ack(uint8_t *buf, MsgInfo *info);

      // Internal helper methods to handle the API cases for cache nodes.
      void cache_node_handle_put(uint8_t *buf, MsgInfo *info);
      void cache_node_handle_put_ack(uint8_t *buf, MsgInfo *info);
      void cache_node_handle_get(uint8_t *buf, MsgInfo *info);
      void cache_node_handle_get_ack(uint8_t *buf, MsgInfo *info);
      void cache_node_handle_push(uint8_t *buf, MsgInfo *info);
      void cache_node_handle_push_ack(uint8_t *buf, MsgInfo *info);

   public:
      // Constructor which associated this Quorum object with its encapsulating
      // node reference object, as well as the type of node this friend is. This
      // is needed so the Quorum object can access its friend's private members.
      Quorum(Node *node, NodeType node_type);

      virtual ~Quorum();

      // Method to handle a PUT message received by either a cache or swing
      // node.
      virtual void handle_put();

      // Method to handle a PUT_ACK message received by either a cache or
      // swing node.
      virtual void handle_put_ack();

      // Method to handle a GET message received by either a cache or swing
      // node.
      virtual void handle_get();

      // Method to handle a GET_ACK message received by either a cache or
      // swing node.
      virtual void handle_get_ack();

      // Method to handle a FORWARD message received by either a cache or
      // swing node.
      virtual void handle_push();

      // Method to handle a FORWARD_ACK message received by either a cache or
      // swing node.
      virtual void handle_push_ack();
};

#endif
