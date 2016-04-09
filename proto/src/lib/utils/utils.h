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
