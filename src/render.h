#ifndef RENDER_H
#define RENDER_H

#include <string>
#include <memory>
#include <ev++.h>

#include "process.h"
#include "widget.h"
#include "widgetregistry.h"

class Render
{
public:
    Render(const ev::loop_ref &loop, std::shared_ptr<WidgetRegistry> reg);

    void operator()(Process *p, int revents);
    void timer_cb(ev::timer &t, int revents);
    void update_cb(ev::async &a, int revents);

    void push_dialog(std::shared_ptr<Widget> w);
    void pop_dialog(Widget* w);

    void update();

    void kill();
private:
    Process _lemonbar;
    ev::timer _timer;
    ev::async _update_trigger;
    std::shared_ptr<WidgetRegistry> _reg;

    std::vector<std::shared_ptr<Widget>> _dialog_stack;

    void draw_standard();
    void draw_dialog();
    void redraw();
};

#endif // RENDER_H
