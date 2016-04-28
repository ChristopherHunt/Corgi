#include <stdio.h>
#include "utility/utils/utils.h"

// Replaces all instances of a comma with a space in a string.
static void replace_commas(std::string &str);

void vector_to_stringlist(std::vector<char> &vec, std::string &result) {
   result.clear();
   std::stringstream ss;
   std::copy(vec.begin(), vec.end(), std::ostream_iterator<char>(ss, " "));
   result.assign(ss.str());
}

void vector_to_stringlist(std::vector<uint32_t> &vec, std::string &result) {
   result.clear();
   std::stringstream ss;
   std::copy(vec.begin(), vec.end(), std::ostream_iterator<uint32_t>(ss, " "));
   result.assign(ss.str());
}

void stringlist_to_vector(std::vector<char> &vec, std::string &result) {
   replace_commas(result);
   vec.clear();
   std::stringstream ss(result);
   std::copy(std::istream_iterator<char>(ss),
         std::istream_iterator<char>(), std::back_inserter(vec));
}

void stringlist_to_vector(std::vector<uint32_t> &vec, std::string &result) {
   replace_commas(result);
   vec.clear();
   std::stringstream ss(result);
   std::copy(std::istream_iterator<uint32_t>(ss),
         std::istream_iterator<uint32_t>(), std::back_inserter(vec));
}

void get_timestamp(uint64_t *timestamp) {
   *timestamp = (uint64_t)(std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
}

static void replace_commas(std::string &str) {
   std::replace(str.begin(), str.end(), ',', ' ');
}
