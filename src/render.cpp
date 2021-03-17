#include <iostream>
#include <vector>
#include <string>
#include <time.h>

#include "render.h"
#include "colors.h"
#include "utils.h"

static const char *lemonbar_args[] = {
    "-f", "Font Awesome 5 Free:size=16",
    "-f", "Font Awesome 5 Brands:size=16",
    "-f", "Font Awesome 5 Free Solid:size=16",
    "-f", "Source Code Pro:size=14",
    "-f", "Roboto Medium:size=16",
    "-B", background,
    "-a", "15",
    nullptr
};

Render::Render(const ev::loop_ref &loop, std::shared_ptr<WidgetRegistry> reg) :
    _lemonbar(loop), _reg(reg)
{
    _lemonbar.set_path("lemonbar");

    int at = 0;
    while (lemonbar_args[at] != nullptr) {
        _lemonbar << lemonbar_args[at];
        at++;
    }

    _lemonbar.set_iocb(this);
    _lemonbar.run();

    _update_trigger.set(loop);
    _update_trigger.set<Render, &Render::update_cb>(this);
    _update_trigger.start();

    _timer.set(loop);
    _timer.set(0, 1);
    _timer.set<Render, &Render::timer_cb>(this);
    _timer.start();
}

void Render::operator()(Process *p, int revents)
{
    auto data = p->read();

    data.pop_back(); // delete the newline

    std::clog << "[lemonbar]: " << data << std::endl;

    if (_dialog_stack.size()) {
        _dialog_stack.back()->handle_command(data);
    } else {
        _reg->route_cmd(data);
    }
}

void Render::timer_cb(ev::timer &t, int revents)
{
    redraw();
}

void Render::update_cb(ev::async &a, int revents)
{
    redraw();
}

void Render::kill()
{
    _lemonbar.kill();
}

void Render::draw_standard()
{
    static const std::string fg_color_val = fg_color(foreground);
    std::stringstream output;

    // left
    output << "%{l}" << fg_color_val
           << _reg->widget("launcher")->render()
           << "  " << _reg->widget("i3ipc")->render()
              ;

    // center
    output << "%{c}" << fg_color_val
           << _reg->widget("datetime")->render();

    // right
    output << "%{r}" << fg_color_val
           << _reg->widget("battery")->render()
           << " " << _reg->widget("disk")->render()
           << " " << _reg->widget("memory")->render()
           << " " << _reg->widget("load")->render()
           << " " << _reg->widget("weather")->render()
           << " "
           << fg_color(foreground) << _reg->widget("power")->render()
              ;

    output << "\n";

    std::string render_str = output.str();

    //std::clog << render_str;

    _lemonbar.write({render_str.begin(), render_str.end()});
}

void Render::draw_dialog()
{
    std::stringstream ss;

    std::shared_ptr<Widget> curr = _dialog_stack.back();

    ss << curr->render();
    ss << "\n";

    _lemonbar.write(ss.str());
}

void Render::redraw()
{
    if (_dialog_stack.size()) {
        draw_dialog();
    } else {
        draw_standard();
    }
}

void Render::push_dialog(std::shared_ptr<Widget> w)
{
    _dialog_stack.push_back(w);
    redraw();
}

void Render::pop_dialog(Widget* w)
{
    for (auto it = _dialog_stack.begin(); it != _dialog_stack.end(); ) {
        if (it->get() == w) {
            it = _dialog_stack.erase(it);
        } else {
            ++it;
        }
    }

    redraw();
}

void Render::update()
{
    _update_trigger.send();
}


