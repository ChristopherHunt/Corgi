#ifndef __CACHE__H__
#define __CACHE__H__

#include <string>

class Cache {
    public:
        Cache();

        ~Cache();

        void get(std::string key);

        void put(std::string key, std::string value);

    private:
};

#endif

