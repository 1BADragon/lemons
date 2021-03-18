#include "widgetregistry.h"

void WidgetRegistry::add_widget(std::shared_ptr<Widget> w)
{
    _widgets[w->name()] = w;

    std::vector<std::string> cmds;
    w->add_commands(cmds);

    for (auto &cmd : cmds) {
        _cmds[cmd] = w;
    }
}

std::shared_ptr<Widget> WidgetRegistry::widget(const std::string &s) const
{
    return _widgets.at(s);
}

bool WidgetRegistry::route_cmd(const std::string &s)
{
    auto n = s.find('-');
    std::string cmd_base;

    if (n == std::string::npos) {
        cmd_base = s;
    } else {
        cmd_base = s.substr(0, n);
    }

    auto w = _cmds.find(cmd_base);

    if (w != _cmds.end()) {
        w->second->handle_command(s);
        return true;
    }

    return false;
}

