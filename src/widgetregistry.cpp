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
    auto w = _cmds.find(s);

    if (w != _cmds.end()) {
        w->second->handle_command(s);
        return true;
    }

    return false;
}

