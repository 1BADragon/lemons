#include <sstream>
#include <fstream>
#include <iomanip>

#include "battery.h"
#include "colors.h"
#include "utils.h"

static const char * status[] = {
    "\uf244", "\uf243", "\uf242",
    "\uf241", "\uf240"
};

static const char * chrg = "\uf1e6";

Battery::Battery(ev::loop_ref &loop)
{
    _timer.set(loop);
    _timer.set(0.0, 10.0);
    _timer.set<Battery, &Battery::timer_cb>(this);
    _timer.start();

    _last_percent = 0.0;
    _last_status = "UNK";
}

std::string Battery::name() const
{
    return "battery";
}

std::string Battery::render() const
{
    std::stringstream ret;

    if (_last_status == "Discharging") {
        if (_last_percent < 5) {
            ret << status[0];
        } else if (_last_percent < 35) {
            ret << status[1];
        } else if (_last_percent < 60) {
            ret << status[2];
        } else if (_last_percent < 80) {
            ret << status[3];
        } else {
            ret << status[4];
        }
    } else {
        ret << chrg;
    }

    ret << " %{F";
    if (_last_percent < 5) {
        ret << bright_red;
    } else if (_last_percent < 30) {
        ret << bright_yellow;
    } else {
        ret << neutral_green;
    }

    ret << "}" << std::fixed << std::setprecision(2) << _last_percent
        << fg_color(foreground) << "%";
    return ret.str();
}

void Battery::timer_cb(ev::timer &t, int revent)
{
    static const std::string base_path = "/sys/class/power_supply/BAT1/";
    double full_cap;
    double current_cap;
    std::string status;

    {
        std::ifstream(base_path + "energy_full") >> full_cap;
        std::ifstream(base_path + "energy_now") >> current_cap;
        std::ifstream(base_path + "status") >> status;
    }

    _last_percent = (current_cap / full_cap) * 100.;
    _last_status = status;
}
