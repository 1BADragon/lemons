#include <sstream>
#include <fstream>
#include <iomanip>

#include "battery.h"
#include "colors.h"
#include "utils.h"

const char * status[] = {
    "\uf244"
};

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

    ret << status[0] << " %{F";

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

    if (status == "Discharging") {
        _last_status = "BAT";
    } else {
        _last_status = "CHR";
    }
}
