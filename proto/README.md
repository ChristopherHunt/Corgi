CORGI API
===============

CORGI is an in-memory distributed MPI cache written in C++. To use CORGI, each
node in a MPI Communicator must instantiate a CORGI object, at which point that
node can participate in the cache communication. Below is a list of the cache
functions available to the user's MPI process:

    // Constructor that takes in the program's argc and argv references. In
    // the cache ecosystem there is cache specific arguments in these command
    // line arguments which get interpretted and striped out prior to
    // returning to the caller.
    Cache(int *argc_ptr, char ***argv_ptr);

    // Adds the tuple described by the key/value pair to the cache, storing
    // the copy on the local cache node as well as updating it within the
    // cache as a whole. Returns true on success.
    bool put(const std::string& key, const std::string& value);

    // Gets the most up-to-date copy of the data corresponding to the input
    // key, returning the result by overwritting the value input. On failure
    // false is returned and the contents of value is set to empty.
    bool get(const std::string& key, std::string& value);

    // Pushes the most current key/value tuple to another node within the cache
    // (the local copy is still retained on the initial node). Returns true on
    // success.
    bool push(const std::string& key, uint32_t node_id);

    // Pulls the most current key/value tuple from the cache to the calling
    // node. Returns true if the pull succeeds.
    bool pull(const std::string& key, uint32_t node_id);

    // Pushes the most current key/value tuple to a set of nodes within the
    // cache. If the scatter does not succeed, false is returned and the
    // contents of the node_ids vector will contain the nodes whom the scatter
    // filed to reach.
    bool scatter(const std::string& key, const std::vector<uint32_t>& node_ids);

    // Pulls a key/value tuple from a set of nodes within the cache (the local
    // copy is still retained on the initial node). If the gather does not
    // succeed, false is returned and the contents of the node_ids vector will
    // contain the nodes whom the gather failed to reach.
    bool gather(const std::string& key, const std::vector<uint32_t>& node_ids);

    // Drops the key/value tuple associated with the input key from the
    // cache. Returns true on success.
    bool drop(const std::string& key);

    // Coalesces all entries for the specified key to the calling node, with
    // the final value being the most current value associated with the key
    // from any of the nodes in the cache. Returns true on success.
    bool collect(const std::string& key);

    // Populates the owners vector with a list of all nodes within the
    // system that have the tuple specified by the input key.
    void get_owners(const std::string& key, std::vector<uint32_t>& owners);

    // Adds the tuple described by the key/value pair to the calling node's
    // local cache node only. Returns true on success.
    bool put_local(const std::string& key, const std::string& value);

    // Gets the value associated with the key from the calling node's local
    // cache node only. On failure, false is returned and the contents of
    // value is set to empty.
    bool get_local(const std::string& key, std::string& value);

    // Pushes a key/value tuple to another node within the cache (the local
    // copy is still retained on the initial node). Returns true on success.
    bool push_local(const std::string& key, uint32_t node_id);

    // Pull a key/value tuple from another node within the cache (the local
    // copy is still retained on the initial node). Returns true if the pull
    // succeeded, false otherwise.
    bool pull_local(const std::string& key, uint32_t node_id);

    // Pushes the calling node's local key/value tuple to a set of nodes
    // within the cache (the local copy is still retained on the initial node).
    // If the scatter does not succeed, false is returned and the contents of
    // the node_ids vector will contain the nodes whom the scatter failed to
    // reach.
    bool scatter_local(const std::string& key, const std::vector<uint32_t>& node_ids);

    // Drops the key/value tuple associated with the input key from the
    // calling node. Returns true on success.
    bool drop_local(const std::string& key);


The below table represents the current state of the Corgi API's function set.
Note that functions marked with a ✕ may be implemented but have not been fully
tested or finalized.

    -------------------------------------
    |   API Functions   |  Implemented  |
    -------------------------------------
    |   put             |       ✕       | <-- Decided to roll back
    |   get             |       ✕       | <-- Decided to roll back
    |   push            |       ✕       | <-- Decided to roll back
    |   pull            |       ✕       |
    |   scatter         |       ✕       |
    |   gather          |       ✕       |
    |   drop            |       ✕       |
    |   collect         |       ✕       |
    |   get_owners      |       ✕       |
    |   put_local       |       ✓       |
    |   get_local       |       ✓       |
    |   push_local      |       ✓       |
    |   pull_local      |       ✕       |
    |   scatter_local   |       ✕       |
    |   drop_local      |       ✕       |
    -------------------------------------

