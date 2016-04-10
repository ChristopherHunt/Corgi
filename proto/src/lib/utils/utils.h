#ifndef __UTILS__H__
#define __UTILS__H__

#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

#define ASSERT(expression, todo) {\
   if (!(expression)) {\
      fprintf(stderr, "\n!!! ASSERT FAILED !!!\n");\
      fprintf(stderr, "\tFile : \"%s\"\n\tFunction : \"%s\"\n\t"\
            "Line : %d\n\n", __FILE__, __func__, __LINE__);\
      todo;\
   }\
}

// Struct to keep track of which communicators are associated with a given job
// number.
// swing -> the swing node communicator for this job
// cache -> the cache node communicator for this job
// job ---> the job node communicator for this job
typedef struct CommGroup {
   MPI_Comm swing;
   MPI_Comm cache;
   MPI_Comm job;
} __attribute__((packed)) Comms;

// Struct to keep track of a job node where:
// job_num ----> the job number this job belongs to
// job_node ---> the node number of the job within its MPI_COMM_WORLD
// cache_node -> the cache node that services this job node.
typedef struct JobNodeID {
   uint32_t job_num;
   uint32_t job_node;
   uint32_t cache_node;

   bool operator== (const JobNodeID& other) {
      return this->job_num == other.job_num &&
         this->job_node == other.job_node;
   }
} __attribute__((packed)) JobNodeID;

// Struct to hold a vote for a quroum poll. This is used when nodes consult one
// another to determine which of their values for a given key is the most
// recent. The struct includes the following fields:
// timestamp -> the timestamp of the voter's value
// value -----> the value being presented for consideration
typedef struct Parcel {
   uint64_t timestamp;
   std::string value;

   bool operator< (const Parcel& other) {
      return this->timestamp < other.timestamp;
   }
} Parcel;

// Converts a vector of characters into a space deliminated string.
void vector_to_stringlist(std::vector<char> &vec, std::string &result);

// Converts a vector of integers into a space deliminated string.
void vector_to_stringlist(std::vector<uint32_t> &vec, std::string &result);

// Convert a list of characters into a vector of characters, breaking each
// character on either a space or a comma.
void stringlist_to_vector(std::vector<char> &vec, std::string &result);

// Convert a list of integers into a vector of characters, breaking each
// character on either a space or a comma.
void stringlist_to_vector(std::vector<uint32_t> &vec, std::string &result);

// Sets the value of timestamp to the current machine wall-clock time.
void get_timestamp(uint64_t *timestamp);

#endif
