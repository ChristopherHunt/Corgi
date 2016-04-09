#ifndef __CACHE__H__
#define __CACHE__H__

#include <stdint.h>
#include <string>
#include <vector>

class Cache {
   private:
      uint32_t job_num;

      int local_size;
      int local_rank;
      int parent_size;
      int parent_rank;
      int coord_rank;

      MPI_Comm parent_comm;

      void allocate();

      void orient(int *argc_ptr, char ***argv_ptr);

      uint8_t *buf;

   public:
      Cache(int *argc_ptr, char ***argv_ptr);

      ~Cache();

      // Adds the tuple described by the key/value pair to the cache, storing
      // the copy on the local cache node as well as updating it within the
      // cache as a whole. Returns true on success.
      bool put(const std::string& key, const std::string& value);

      // Gets the most up-to-date copy of the data corresponding to the input
      // key, returning the result by overwritting the value input. On failure
      // false is returned and the contents of value is set to empty.
      bool get(const std::string& key, std::string& value);

      // Pushes a key/value tuple to another node within the cache (the local
      // copy is still retained on the initial node). Returns true on success.
      bool push(const std::string& key, uint32_t node_id);

      // Pull a key/value tuple from another node within the cache (the local
      // copy is still retained on the initial node). Returns true if the pull
      // succeeded, false otherwise.
      bool pull(const std::string& key, uint32_t node_id);

      // Pushes a key/value tuple to a set of nodes within the cache (the local
      // copy is still retained on the initial node). If the scatter does not
      // succeed, false is returned and the contents of the node_ids vector will
      // contain the nodes whom the scatter failed to reach.
      bool scatter(const std::string& key,
         const std::vector<uint32_t>& node_ids);

      // Pulls a key/value tuple from a set of nodes within the cache (the local
      // copy is still retained on the initial node). If the gather does not
      // succeed, false is returned and the contents of the node_ids vector will
      // contain the nodes whom the gather failed to reach.
      bool gather(const std::string& key, 
         const std::vector<uint32_t>& node_ids);

      // Drops the key/value tuple associated with the input key from the
      // cache. Returns true on success.
      bool drop(const std::string& key);

      // Coalesces all entries for the specified key to the calling node, with
      // the final value being the most current value associated with the key
      // from any of the nodes in the cache. Returns true on success.
      bool collect(const std::string& key);

      // Adds the tuple described by the key/value pair to the calling node's
      // local cache node only. Returns true on success.
      bool put_local(const std::string& key, const std::string& value);

      // Gets the value associated with the key from the calling node's local
      // cache node only. On failure, false is returned and the contents of
      // value is set to empty.
      bool get_local(const std::string& key, std::string& value);

      // Populates the owners vector with a list of all nodes within the
      // system that have the tuple specified by the input key.
      void get_owners(const std::string& key, std::vector<uint32_t>& owners);
};

#endif

