#include "launcher.h"

Launcher::Launcher()
{

}

std::string Launcher::name() const
{
    return "launcher";
}

std::string Launcher::render() const
{
    return "\uf17c";
}
