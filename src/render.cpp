#include <iostream>
#include <vector>
#include <string>
#include <time.h>

#include "render.h"
#include "colors.h"
#include "utils.h"

static const char *lemonbar_args[] = {
    "lemonbar",
    "-f", "Font Awesome 5 Free:size=16",
    "-f", "Font Awesome 5 Brands:size=16",
    "-f", "Font Awesome 5 Free Solid:size=16",
    "-f", "Source Code Pro:size=14",
    "-f", "Roboto Medium:size=16",
    "-B", background,
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

    _reg->route_cmd(data);
}

void Render::timer_cb(ev::timer &t, int revents)
{
    static const std::string fg_color_val = fg_color(foreground);
    std::stringstream output;

    // left
    output << "%{l}" << fg_color_val
           << _reg->widget("launcher")->render();
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

void Render::kill()
{
    _lemonbar.kill();
}


