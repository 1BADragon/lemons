#ifndef WIDGETREGISTRY_H
#define WIDGETREGISTRY_H

#include <unordered_map>
#include <memory>
#include <string>

#include "widget.h"

class WidgetRegistry
{
public:
    void add_widget(std::shared_ptr<Widget> w);
    std::shared_ptr<Widget> widget(const std::string &s) const;

    bool route_cmd(const std::string &s);

private:
    std::unordered_map<std::string, std::shared_ptr<Widget>> _widgets;
    std::unordered_map<std::string, std::shared_ptr<Widget>> _cmds;
};

#endif // WIDGETREGISTRY_H
