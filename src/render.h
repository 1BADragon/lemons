#ifndef RENDER_H
#define RENDER_H

#include <string>
#include <memory>
#include <ev++.h>

#include "process.h"
#include "widgetregistry.h"

class Render
{
public:
    Render(const ev::loop_ref &loop, std::shared_ptr<WidgetRegistry> reg);

    void operator()(Process *p, int revents);
    void timer_cb(ev::timer &t, int revents);
private:
    Process _lemonbar;
    ev::timer _timer;
    std::shared_ptr<WidgetRegistry> _reg;
};

#endif // RENDER_H
