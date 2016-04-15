#ifndef __LOGGER__H__
#define __LOGGER__H__

#include <string>

#define OPEN_FAILURE -2
#define WRITE_FAILURE -1

class Logger {
   private:
      int fd;
      std::string filename;

   public:

      Logger(const std::string& log_file);

      ~Logger();

      int log(const std::string& text);

      int view(std::string& output);

      int clear();

      int get_fd();

      void get_filename(std::string& output);
};

#endif
