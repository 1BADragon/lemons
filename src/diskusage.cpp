#include <sys/vfs.h>
#include <sstream>
#include <iomanip>

#include "diskusage.h"
#include "colors.h"
#include "utils.h"

static const char *icon = "\uf0a0";

DiskUsage::DiskUsage(ev::loop_ref loop, const std::string &path) :
    Widget(), _path(path), _timer(loop)
{
    _timer.set<DiskUsage, &DiskUsage::timer_cb>(this);
    _timer.start(0.0, 10.0);
}

void DiskUsage::timer_cb(ev::timer &t, int revents)
{
    struct statfs s;

    if (statfs(_path.c_str(), &s)) {
        throw Widget::Error("Unable to call statfs", errno);
    }

    _last_percent = (static_cast<double>(s.f_bfree) /
                     static_cast<double>(s.f_blocks)) * 100.;

}

std::string DiskUsage::render() const
{
    std::stringstream ss;

    ss << icon << " ";

    if (_last_percent < 10.) {
        ss << fg_color(neutral_red);
    } else if (_last_percent < 30.) {
        ss << fg_color(neutral_orange);
    } else {
        ss << fg_color(neutral_green);
    }

    ss << std::fixed << std::setprecision(2) << _last_percent
       << fg_color(foreground) << "%";
    return ss.str();
}

std::string DiskUsage::name() const
{
    return "disk";
}
