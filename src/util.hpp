#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <string>


namespace util
{
    unsigned long make_seed();
    std::string trim(std::string const& str, std::string const & whitespace = " \t");
}

#endif // __UTIL_HPP__
