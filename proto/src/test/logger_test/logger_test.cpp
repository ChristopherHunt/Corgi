#include <fcntl.h>
#include <string.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <stdint.h>
#include "utility/logger/logger.h"

TEST(loggerTest, logCreation) {
   std::string filename = "test.log";
   std::string logger_filename;

   Logger *logger = new Logger(filename);
   logger->get_filename(logger_filename);

   EXPECT_TRUE(logger->get_fd() >= 0);
   EXPECT_STREQ(filename.c_str(), logger_filename.c_str());

   delete(logger);
}

TEST(loggerTest, logWrite) {
   int result;
   int fd;
   char buf[16] = {0};

   std::string filename = "logWrite.log";
   std::string log_msg = "some log msg";

   Logger *logger = new Logger(filename);
   result = logger->log(log_msg);
   EXPECT_EQ(result, log_msg.size());

   fd = open(filename.c_str(), O_RDONLY, 0444);
   ASSERT_TRUE(fd >= 0);

   result = read(fd, buf, log_msg.size());
   EXPECT_EQ(result, log_msg.size());
   EXPECT_STREQ(buf, log_msg.c_str());

   delete(logger);
   close(fd);
}

TEST(loggerTest, logAppend) {
   int result;
   int fd;
   char buf[32] = {0};

   std::string filename = "logAppend.log";
   std::string log_msg_1 = "first log msg\n";
   std::string log_msg_2 = "second log msg";
   std::string final_msg = "first log msg\nsecond log msg";

   Logger *logger = new Logger(filename);
   result = logger->log(log_msg_1);
   EXPECT_EQ(result, log_msg_1.size());

   result = logger->log(log_msg_2);
   EXPECT_EQ(result, log_msg_2.size());

   fd = open(filename.c_str(), O_RDONLY, 0444);
   ASSERT_TRUE(fd >= 0);

   result = read(fd, buf, final_msg.size());
   EXPECT_EQ(result, final_msg.size());
   EXPECT_STREQ(buf, final_msg.c_str());

   delete(logger);
   close(fd);
}


TEST(loggerTest, logClear) {
   int result;
   int fd;
   char buf[16] = {0};

   std::string filename = "logClear.log";
   std::string log_msg = "some log msg";

   Logger *logger = new Logger(filename);
   result = logger->log(log_msg);
   EXPECT_EQ(result, log_msg.size());

   fd = open(filename.c_str(), O_RDONLY, 0444);
   ASSERT_TRUE(fd >= 0);

   result = read(fd, buf, log_msg.size());
   EXPECT_EQ(result, log_msg.size());
   EXPECT_STREQ(buf, log_msg.c_str());
   close(fd);

   logger->clear();

   fd = open(filename.c_str(), O_RDONLY, 0444);
   ASSERT_TRUE(fd >= 0);

   memset(buf, '\0', 16);
   result = read(fd, buf, log_msg.size());
   EXPECT_STREQ(buf, "");

   delete(logger);
   close(fd);
}

TEST(loggerTest, logView) {
   int result;

   std::string filename = "logView.log";
   std::string log_msg = "some log msg";
   std::string content;

   Logger *logger = new Logger(filename);
   result = logger->log(log_msg);
   EXPECT_EQ(result, log_msg.size());

   result = logger->view(content);
   EXPECT_EQ(result, log_msg.size());
   EXPECT_STREQ(content.c_str(), log_msg.c_str());

   delete(logger);
}

// Run the test driver.
int main(int argc, char** argv) {
   testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
