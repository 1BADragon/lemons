#include <iostream>
#include <memory>
#include <sstream>

#include "process.h"
#include "power.h"
#include "utils.h"

Power::Power(ev::loop_ref &loop) :
    _loop(loop)
{

}

std::string Power::name() const
{
    return "power";
}

std::string Power::render() const
{
    return "%{A:power:}\uf011%{A}";
}

bool Power::add_commands(std::vector<std::string> &cmds) const
{
    cmds.push_back("power");

    return true;
}

bool Power::handle_command(const std::string &cmd)
{
    if (cmd == "power") {
        std::clog << "Got power command" << std::endl;

        auto power_dialog = std::make_shared<Dialog>(_loop);

        ::render()->push_dialog(power_dialog);

        return true;
    }

    return false;
}


Power::Dialog::Dialog(ev::loop_ref &loop) :
    _timer(loop), _loop(loop)
{
    _timer.set<Power::Dialog, &Power::Dialog::timer_cb>(this);
    _timer.set(5.0, 0.0);
    _timer.start();
}

std::string Power::Dialog::name() const
{
    return "power::dialog";
}

std::string Power::Dialog::render() const
{
    std::stringstream ss;

    ss << "%{r}"
       << button("Power Off", "poweroff", dark0_hard, foreground)
       << "  "
       << button("Reboot", "reboot", dark0_hard, foreground)
       << "  "
       << button("Suspend", "suspend", dark0_hard, foreground)
          ;

    log() << ss.str() << std::endl;

    return ss.str();
}

bool Power::Dialog::handle_command(const std::string &cmd)
{
    if (cmd == "poweroff") {
        Process::exec(_loop, "poweroff");
    } else if (cmd == "reboot") {
        Process::exec(_loop, "reboot");
    } else if (cmd == "suspend") {
        Process::exec(_loop, "systemctl", {"suspend"});
    } else {
        return false;
    }

    ::render()->pop_dialog(this);

    return true;
}

void Power::Dialog::timer_cb(ev::timer &t, int revents)
{
    t.stop();
    ::render()->pop_dialog(this);
}
