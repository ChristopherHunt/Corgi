#include <stdio.h>
#include <string.h>
#include <string>
#include "network.h"
#include "utility/utils/utils.h"

void print_msg_info(MsgInfo *msg_info) {
   MsgTag tag = (MsgTag)msg_info->tag;
   printf("===== MsgInfo =====\n");
   printf("tag ---------> %s\n", msg_tag_handle(tag));
   printf("src ---------> %d\n", msg_info->src);
   printf("count (bytes): %d\n", msg_info->count);

   if (msg_info->comm == MPI_COMM_WORLD) {
      printf("comm: MPI_COMM_WORLD\n");
   }
   else {
      printf("comm: OTHER\n");
   }
}

const char * msg_tag_handle(MsgTag tag) {
   switch (tag) {
      case PUT:
         return "PUT";
      case PUT_ACK:
         return "PUT_ACK";
      case PUT_LOCAL:
         return "PUT_LOCAL";
      case PUT_LOCAL_ACK:
         return "PUT_LOCAL_ACK";
      case GET:
         return "GET";
      case GET_ACK:
         return "GET_ACK";
      case GET_LOCAL:
         return "GET_LOCAL";
      case GET_LOCAL_ACK:
         return "GET_LOCAL_ACK";
      case PUSH:
         return "PUSH";
      case PUSH_ACK:
         return "PUSH_ACK";
      case PUSH_LOCAL:
         return "PUSH_LOCAL";
      case PUSH_LOCAL_ACK:
         return "PUSH_LOCAL_ACK";
      case PULL:
         return "PULL";
      case PULL_ACK:
         return "PULL_ACK";
      case PULL_LOCAL:
         return "PULL_LOCAL";
      case PULL_LOCAL_ACK:
         return "PULL_LOCAL_ACK";
      case DROP:
         return "DROP";
      case DROP_ACK:
         return "DROP_ACK";
      case DROP_LOCAL:
         return "DROP_LOCAL";
      case DROP_LOCAL_ACK:
         return "DROP_LOCAL_ACK";
      case SPAWN_JOB:
         return "SPAWN_JOB";
      case SPAWN_CACHE:
         return "SPAWN_CACHE";
      case EXIT:
         return "EXIT";
      default:
         return "UNKNOWN FLAG!";
   }
}

int send_msg(const void *buf, int count, MPI_Datatype datatype, int dest,
      int tag, MPI_Comm comm, MPI_Request *request) {
#ifdef DEBUG
   printf("send_msg:\n");
   printf("\ttag: %s\n", msg_tag_handle((MsgTag)tag));
   printf("\tdest: %d\n", dest);
#endif

   int result;

   result = MPI_Isend(buf, count, datatype, dest, tag, comm, request);
   wait_for_send(request);
   return result;
}

int recv_msg(void *buf, int count, MPI_Datatype datatype, int source, int tag,
      MPI_Comm comm, MPI_Status *status) {
   return MPI_Recv(buf, count, datatype, source, tag, comm, status);
}

void wait_for_send(MPI_Request *request) {
   int flag = 0;
   MPI_Status status;
   while (flag == 0) {
      MPI_Test(request, &flag, &status);
   }
}

SpawnNodesTemp::SpawnNodesTemp() :
   job_num(0), count(0), mapping_size(0), mapping(""), exec_size(0),
   exec_name("") {
   }

SpawnNodesTemp::~SpawnNodesTemp() {
}

void SpawnNodesTemp::pack(SpawnNodesTemplate *temp, uint32_t job_num,
      uint16_t count, const std::string& mapping, const std::string &exec_name) {
   MPI_ASSERT(temp != NULL);

   this->job_num = job_num;
   temp->job_num = job_num;

   this->count = count;
   temp->count = count;

   this->mapping_size = (uint16_t)mapping.size();
   temp->mapping_size = (uint16_t)mapping.size();

   this->mapping.assign(mapping);
   memcpy(temp->mapping, mapping.c_str(), mapping_size);

   this->exec_size = exec_name.size();
   temp->exec_size = exec_name.size();

   this->exec_name.assign(exec_name);
   memcpy(temp->exec_name, exec_name.c_str(), exec_size);
}

void SpawnNodesTemp::unpack(SpawnNodesTemplate *temp) {

   MPI_ASSERT(temp != NULL);

   job_num = temp->job_num;
   count = temp->count;
   mapping_size = temp->mapping_size;
   mapping.assign(std::string(temp->mapping, temp->mapping + mapping_size));
   exec_size = temp->exec_size;
   exec_name.assign(std::string(temp->exec_name, temp->exec_name + exec_size));
}

PutTemp::PutTemp() :
   job_num(0), job_node(0), cache_node(0), key_size(0), key(""), value(""),
   timestamp(0) {
   }

PutTemp::~PutTemp() {
}

void PutTemp::pack(PutTemplate *temp, uint32_t job_num, uint32_t job_node,
      uint32_t cache_node, const std::string& key, const std::string& value,
      uint64_t timestamp) {

   MPI_ASSERT(temp != NULL);

   this->job_num = job_num;
   temp->job_num = job_num;

   this->job_node = job_node;
   temp->job_node = job_node;

   this->cache_node = cache_node;
   temp->cache_node = cache_node;

   this->key_size = key.size();
   temp->key_size = key.size();

   this->key.assign(std::string(key));
   memcpy(temp->key, key.c_str(), key.size());

   this->value_size = value.size();
   temp->value_size = value.size();

   this->value.assign(value);
   memcpy(temp->value, value.c_str(), value.size());

   this->timestamp = timestamp;
   temp->timestamp = timestamp;
}

void PutTemp::unpack(PutTemplate *temp) {
   MPI_ASSERT(temp != NULL);

   job_num = temp->job_num;
   job_node = temp->job_node;
   cache_node = temp->cache_node;
   key_size = temp->key_size;
   key.assign(std::string(temp->key, temp->key + key_size));
   value_size = temp->value_size;
   value.assign(std::string(temp->value, temp->value + value_size));
   timestamp = temp->timestamp;
}

PutAckTemp::PutAckTemp() :
   job_num(0), job_node(0), cache_node(0), key_size(0), key(""), result(0) {
   }

PutAckTemp::~PutAckTemp() {
}

void PutAckTemp::pack(PutAckTemplate *temp, uint32_t job_num, uint32_t job_node,
      uint32_t cache_node, const std::string& key, uint8_t result) {

   MPI_ASSERT(temp != NULL);

   this->job_num = job_num;
   temp->job_num = job_num;

   this->job_node = job_node;
   temp->job_node = job_node;

   this->cache_node = cache_node;
   temp->cache_node = cache_node;

   this->key_size = key.size();
   temp->key_size = key.size();

   this->key.assign(key);
   memcpy(temp->key, key.c_str(), key.size());

   this->result = result;
   temp->result = result;
}

void PutAckTemp::unpack(PutAckTemplate *temp) {
   MPI_ASSERT(temp != NULL);

   job_num = temp->job_num;
   job_node = temp->job_node;
   cache_node = temp->cache_node;
   key_size = temp->key_size;
   key.assign(std::string(temp->key, temp->key + key_size));
   result = temp->result;
}

GetTemp::GetTemp() :
   job_num(0), job_node(0), key_size(0), key(""), timestamp(0) {
   }

GetTemp::~GetTemp() {
}

void GetTemp::pack(GetTemplate *temp, uint32_t job_num, uint32_t job_node,
      const std::string& key, uint64_t timestamp) {
   MPI_ASSERT(temp != NULL);

   this->job_num = job_num;
   temp->job_num = job_num;

   this->job_node = job_node;
   temp->job_node = job_node;

   this->key_size = key.size();
   temp->key_size = key.size();

   this->key.assign(key);
   memcpy(temp->key, key.c_str(), key.size());

   this->timestamp = timestamp;
   temp->timestamp = timestamp;
}

void GetTemp::unpack(GetTemplate *temp) {
   MPI_ASSERT(temp != NULL);

   job_num = temp->job_num;
   job_node = temp->job_node;
   key_size = temp->key_size;
   key.assign(std::string(temp->key, temp->key + key_size));
   timestamp= temp->timestamp;
}

GetAckTemp::GetAckTemp() :
   job_num(0), job_node(0), key_size(0), key(""), value_size(0), value(""),
   timestamp(0), result(0) {
   }

GetAckTemp::~GetAckTemp() {
}

void GetAckTemp::pack(GetAckTemplate *temp, uint32_t job_num, uint32_t job_node,
      const std::string& key, const std::string& value, uint64_t timestamp,
      uint8_t result) {
   MPI_ASSERT(temp != NULL);

   this->job_num = job_num;
   temp->job_num = job_num;

   this->job_node = job_node;
   temp->job_node = job_node;

   this->key_size = key.size();
   temp->key_size = key.size();

   this->key.assign(key);
   memcpy(temp->key, key.c_str(), key.size());

   this->value_size = value.size();
   temp->value_size = value.size();

   this->value.assign(value);
   memcpy(temp->value, value.c_str(), value.size());

   this->timestamp = timestamp;
   temp->timestamp = timestamp;

   this->result = result;
   temp->result = result;
}

void GetAckTemp::unpack(GetAckTemplate *temp) {
   MPI_ASSERT(temp != NULL);

   job_num = temp->job_num;
   job_node = temp->job_node;
   key_size = temp->key_size;
   key.assign(std::string(temp->key, temp->key + key_size));
   value_size = temp->value_size;
   value.assign(std::string(temp->value, temp->value + value_size));
   timestamp = temp->timestamp;
   result = temp->result;
}

CensusTemp::CensusTemp() :
   job_num(0), job_node(0), key_size(0), key(""), votes_req(0) {
   }

CensusTemp::~CensusTemp() {
}

void CensusTemp::pack(CensusTemplate *temp, uint32_t job_num, uint32_t job_node, 
      const std::string& key, uint32_t votes_req) {
   MPI_ASSERT(temp != NULL);

   this->job_num = job_num;
   temp->job_num = job_num;

   this->job_node = job_node;
   temp->job_node = job_node;

   this->key_size = key.size();
   temp->key_size = key.size();

   this->key.assign(key);
   memcpy(temp->key, key.c_str(), key.size());

   this->votes_req = votes_req;
   temp->votes_req = votes_req;
}

void CensusTemp::unpack(CensusTemplate *temp) {
   MPI_ASSERT(temp != NULL);

   job_num = temp->job_num;
   job_node = temp->job_node;
   key_size = temp->key_size;
   key.assign(std::string(temp->key, temp->key + key_size));
   votes_req = temp->votes_req;
}

PushTemp::PushTemp() :
   job_num(0), job_node(0), cache_node(0), key_size(0), key("") {
   }

PushTemp::~PushTemp() {
}

void PushTemp::pack(PushTemplate *temp, uint32_t job_num, uint32_t job_node,
      uint32_t cache_node, const std::string& key) {

   MPI_ASSERT(temp != NULL);

   this->job_num = job_num;
   temp->job_num = job_num;

   this->job_node = job_node;
   temp->job_node = job_node;

   this->cache_node = cache_node;
   temp->cache_node = cache_node;

   this->key_size = key.size();
   temp->key_size = key.size();

   this->key.assign(std::string(key));
   memcpy(temp->key, key.c_str(), key.size());
}

void PushTemp::unpack(PushTemplate *temp) {
   MPI_ASSERT(temp != NULL);

   job_num = temp->job_num;
   job_node = temp->job_node;
   cache_node = temp->cache_node;
   key_size = temp->key_size;
   key.assign(std::string(temp->key, temp->key + key_size));
}

PushAckTemp::PushAckTemp() :
   job_num(0), job_node(0), cache_node(0), key_size(0), key(""), result(0) {
   }

PushAckTemp::~PushAckTemp() {
}

void PushAckTemp::pack(PushAckTemplate *temp, uint32_t job_num,
      uint32_t job_node, uint32_t cache_node, const std::string& key,
      uint8_t result) {

   MPI_ASSERT(temp != NULL);

   this->job_num = job_num;
   temp->job_num = job_num;

   this->job_node = job_node;
   temp->job_node = job_node;

   this->cache_node = cache_node;
   temp->cache_node = cache_node;

   this->key_size = key.size();
   temp->key_size = key.size();

   this->key.assign(key);
   memcpy(temp->key, key.c_str(), key.size());

   this->result = result;
   temp->result = result;
}

void PushAckTemp::unpack(PushAckTemplate *temp) {
   MPI_ASSERT(temp != NULL);

   job_num = temp->job_num;
   job_node = temp->job_node;
   cache_node = temp->cache_node;
   key_size = temp->key_size;
   key.assign(std::string(temp->key, temp->key + key_size));
   result = temp->result;
}
