#include <sstream>
#include <fstream>
#include <iomanip>
#include "cpuusage.h"
#include "utils.h"
#include "colors.h"

static const char *icon = "\uf2db";

CpuUsage::CpuUsage(ev::loop_ref loop) :
    Widget(), _timer(loop), _prev_total(0), _prev_idle(0)
{
    _timer.set<CpuUsage, &CpuUsage::timer_cb>(this);
    _timer.start(0, 2.0);
}

void CpuUsage::timer_cb(ev::timer &t, int revents)
{
    std::string junk;
    uint64_t user;
    uint64_t nice;
    uint64_t sys;
    uint64_t idle;
    uint64_t iowait;
    uint64_t irq;
    uint64_t softirq;

    std::ifstream f("/proc/stat");

    f >> junk >> user >> nice >> sys >> idle
            >> iowait >> irq >> softirq;

    uint64_t t_idle = idle + iowait;
    uint64_t t_non_idle = user + nice + sys + irq + softirq;

    uint64_t total = t_idle + t_non_idle;

    uint64_t totald = total - _prev_total;
    uint64_t idled = t_idle - _prev_idle;

    double cpu_load = (static_cast<double>(totald) - static_cast<double>(idled)) / static_cast<double>(totald);

    _prev_total = total;
    _prev_idle = t_idle;

    _last_percent = cpu_load * 100;
}

std::string CpuUsage::name() const
{
    return "load";
}

std::string CpuUsage::render() const
{
    std::stringstream ss;

    ss << icon << " ";

    if (_last_percent > 90) {
        ss << fg_color(bright_red);
    } else if(_last_percent > 75) {
        ss << fg_color(bright_orange);
    } else {
        ss << fg_color(bright_green);
    }

    ss << std::fixed << std::setprecision(2)
       << _last_percent << fg_color(foreground)
       << "%";

    return ss.str();
}
