#include <time.h>
#include <sstream>

#include "datetime.h"
#include "colors.h"


std::string DateTime::render() const
{
    char buf[256];
    time_t current_time = time(nullptr);
    tm *local_time = localtime(&current_time);

    strftime(buf, 256, "%a, %b %e %l:%M:%S%P", local_time);

    std::stringstream ret;
    ret << buf;

    return ret.str();
}

std::string DateTime::name() const
{
    return "datetime";
}
