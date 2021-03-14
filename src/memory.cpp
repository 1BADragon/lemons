#include <sstream>
#include <fstream>
#include <iomanip>

#include "memory.h"
#include "colors.h"
#include "utils.h"


Memory::Memory(ev::loop_ref &loop) :
    _timer(loop)
{
    _timer.set<Memory, &Memory::timer_cb>(this);
    _timer.start(0., 5.0);
}

void Memory::timer_cb(ev::timer &t, int revents)
{
    double total;
    double available;
    std::string trash;

    std::ifstream mem_file("/proc/meminfo");

    mem_file >> trash >> total >> trash
             >> trash >> trash >> trash
             >> trash >> available >> trash;

    _last_percent = ((total - available) / total) * 100;
}

std::string Memory::name() const
{
    return "memory";
}

std::string Memory::render() const
{
    std::stringstream ss;
    ss << "Mem: ";

    if (_last_percent < 75) {
        ss << fg_color(bright_green);
    } else if (_last_percent < 90) {
        ss << fg_color(bright_orange);
    } else {
        ss << fg_color(bright_red);
    }

    ss << std::fixed << std::setprecision(2)
       << _last_percent << fg_color(foreground) << "%";
    return ss.str();
}
