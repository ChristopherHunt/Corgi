#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include "utility/logger/logger.h"

#define FILE_PERMISSIONS 0644

Logger::Logger(const std::string& log_file) {
   filename = log_file;
   fd = open(filename.c_str(), O_CREAT | O_RDWR, FILE_PERMISSIONS);
}

Logger::~Logger() {
   close(fd);
}

int Logger::log(const std::string& text) {
   if (fd < 0) {
      return OPEN_FAILURE;
   }

   return write(fd, text.c_str(), text.size()); 
}

int Logger::view(std::string& output) {
   if (fd < 0) {
      return OPEN_FAILURE;
   }

   std::ifstream ifs(filename);
   output.assign((std::istreambuf_iterator<char>(ifs)),
         (std::istreambuf_iterator<char>()));

   return output.size();
}

int Logger::clear() {
   close(fd);
   fd = open(filename.c_str(), O_CREAT | O_RDWR | O_TRUNC, FILE_PERMISSIONS);
   return fd;
}

int Logger::get_fd() {
   return fd;
}

void Logger::get_filename(std::string& output) {
   output.assign(filename);
}
