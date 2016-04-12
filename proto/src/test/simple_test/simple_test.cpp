#include <gtest/gtest.h>

int factorial(int n) {
   if (n > 1) {
      return n * factorial(n - 1);
   }
   return 1;
}

// Tests factorial of 0.
TEST(factorialTest, HandlesZeroInput) {
   EXPECT_EQ(1, factorial(0));
}

// Tests factorial of positive numbers.
TEST(factorialTest, HandlesPositiveInput) {
   EXPECT_EQ(1, factorial(1));
   EXPECT_EQ(2, factorial(2));
   EXPECT_EQ(6, factorial(3));
   EXPECT_EQ(40320, factorial(8));
}

int main(int argc, char** argv) {
   testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
