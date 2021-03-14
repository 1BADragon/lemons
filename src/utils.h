#ifndef UTILS_H
#define UTILS_H

#include <fstream>
#include <string>
#include <sstream>

static inline std::string fg_color(const std::string &c)
{
    std::stringstream ss;
    ss << "%{F" << c << "}";
    return ss.str();
}

static inline std::string bg_color(const std::string &c)
{
    std::stringstream ss;
    ss << "%{B" << c << "}";
    return ss.str();
}

#endif // UTILS_H
