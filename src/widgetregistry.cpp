#include "widgetregistry.h"

void WidgetRegistry::add_widget(std::shared_ptr<Widget> w)
{
    _widgets[w->name()] = w;
}

std::shared_ptr<Widget> WidgetRegistry::widget(const std::string &s) const
{
    return _widgets.at(s);
}

