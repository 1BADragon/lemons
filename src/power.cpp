#include <iostream>

#include "power.h"

Power::Power()
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

        return true;
    }

    return false;
}
