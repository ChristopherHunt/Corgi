#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <stdint.h>
#include "utils/utils.h"

// Test vector_to_stringlist for char vector
TEST(vectorToStringlistTest, charVector) {
   std::string result;
   std::vector<char> vec;
   vec.push_back('a');
   vec.push_back('b');
   vec.push_back('c');
   vector_to_stringlist(vec, result);
   EXPECT_STREQ("a b c ", result.c_str());
}

// Test vector_to_stringlist for uint32_t vector
TEST(vectorToStringlistTest, uint32Vector) {
   std::string result;
   std::vector<uint32_t> vec;
   vec.push_back(0);
   vec.push_back(1);
   vec.push_back(2);
   vec.push_back(3);
   vector_to_stringlist(vec, result);
   EXPECT_STREQ("0 1 2 3 ", result.c_str());

   vec.clear();
   vec.push_back(0);
   vec.push_back(1);
   vec.push_back(2);
   vec.push_back(13);
   vec.push_back(21);
   vec.push_back(7);
   vector_to_stringlist(vec, result);
   EXPECT_STREQ("0 1 2 13 21 7 ", result.c_str());
}

// Test stringlist_to_vector for uint32_t's for space deliminated strings
TEST(stirnglistToVectorTest, spaceDelimString) {
   std::vector<uint32_t> datum_vec;
   datum_vec.push_back(0);
   datum_vec.push_back(1);
   datum_vec.push_back(2);

   std::string temp("0 1 2");
   std::vector<uint32_t> test_vec;
   stringlist_to_vector(test_vec, temp);

   ASSERT_EQ(datum_vec.size(), test_vec.size());
   for (int i = 0; i < datum_vec.size(); ++i) {
      EXPECT_EQ(datum_vec[i], test_vec[i]);
   }

   datum_vec.clear();
   datum_vec.push_back(0);
   datum_vec.push_back(1);
   datum_vec.push_back(2);
   datum_vec.push_back(13);
   datum_vec.push_back(21);
   datum_vec.push_back(7);

   temp.clear();
   temp.assign("0 1 2 13 21 7");
   stringlist_to_vector(test_vec, temp);

   ASSERT_EQ(datum_vec.size(), test_vec.size());
   for (int i = 0; i < datum_vec.size(); ++i) {
      EXPECT_EQ(datum_vec[i], test_vec[i]);
   }
}

// Test stringlist_to_vector for uint32_t's for comma deliminated strings
TEST(stirnglistToVectorTest, commaDelimString) {
   std::vector<uint32_t> datum_vec;
   datum_vec.push_back(0);
   datum_vec.push_back(1);
   datum_vec.push_back(2);

   std::string temp("0,1,2,");
   std::vector<uint32_t> test_vec;
   stringlist_to_vector(test_vec, temp);

   ASSERT_EQ(datum_vec.size(), test_vec.size());
   for (int i = 0; i < datum_vec.size(); ++i) {
      EXPECT_EQ(datum_vec[i], test_vec[i]);
   }

   temp.clear();
   temp.assign("0,1,2");
   test_vec.clear();
   stringlist_to_vector(test_vec, temp);

   ASSERT_EQ(datum_vec.size(), test_vec.size());
   for (int i = 0; i < datum_vec.size(); ++i) {
      EXPECT_EQ(datum_vec[i], test_vec[i]);
   }
}

// Run the test driver.
int main(int argc, char** argv) {
   testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
