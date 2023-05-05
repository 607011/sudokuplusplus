#include <ctime>

#ifdef __GNUC__
#include <unistd.h>
#endif

#ifdef _MSC_VER
#include <process.h>
#endif

#include "util.hpp"

#include <random>

namespace util
{

    unsigned long make_seed()
    {
        auto a = static_cast<unsigned long>(time(nullptr));
        auto b = static_cast<unsigned long>(clock());
#ifdef _MSC_VER
        auto c = static_cast<unsigned long>(_getpid());
#else
        auto c = static_cast<unsigned long>(getpid());
#endif
        a = a - b;
        a = a - c;
        a = a ^ (c >> 13);
        b = b - c;
        b = b - a;
        b = b ^ (a << 8);
        c = c - a;
        c = c - b;
        c = c ^ (b >> 13);
        a = a - b;
        a = a - c;
        a = a ^ (c >> 12);
        b = b - c;
        b = b - a;
        b = b ^ (a << 16);
        c = c - a;
        c = c - b;
        c = c ^ (b >> 5);
        a = a - b;
        a = a - c;
        a = a ^ (c >> 3);
        b = b - c;
        b = b - a;
        b = b ^ (a << 10);
        c = c - a;
        c = c - b;
        c = c ^ (b >> 15);
        std::random_device detrng;
        return c + detrng();
    }

    std::string trim(std::string const &str, std::string const &whitespace)
    {
        auto start = str.find_first_not_of(whitespace);
        if (start == std::string::npos)
        {
            return std::string();
        }
        auto end = str.find_last_not_of(whitespace);
        auto actual_range = end - start + 1;
        return str.substr(start, actual_range);
    }

}
