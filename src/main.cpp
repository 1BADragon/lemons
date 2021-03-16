#include <iostream>
#include <functional>
#include <memory>
#include <unistd.h>
#include <sys/inotify.h>

#include <ev++.h>

#include "widgetregistry.h"
#include "render.h"
#include "datetime.h"
#include "battery.h"
#include "diskusage.h"
#include "memory.h"
#include "cpuusage.h"
#include "weather.h"
#include "power.h"
#include "launcher.h"
#include "i3ipc.h"

struct Interrupt {
    void operator()(ev::sig &sig, int event)
    {
        sig.loop.break_loop(ev::ALL);
    }
};

static std::shared_ptr<Render> g_render;

std::shared_ptr<Render> render()
{
    return g_render;
}

int main(int argc, char **argv)
{
    auto loop = ev::get_default_loop();

    ev::sig sig_watcher(loop);
    auto int_handle = std::make_shared<Interrupt>();
    sig_watcher.set(SIGINT);
    sig_watcher.set(int_handle.get());
    sig_watcher.start();

    auto reg = std::make_shared<WidgetRegistry>();

    reg->add_widget(std::make_shared<DateTime>());
    reg->add_widget(std::make_shared<Battery>(loop));
    reg->add_widget(std::make_shared<DiskUsage>(loop));
    reg->add_widget(std::make_shared<Memory>(loop));
    reg->add_widget(std::make_shared<CpuUsage>(loop));
    reg->add_widget(std::make_shared<Weather>(loop));
    reg->add_widget(std::make_shared<Power>(loop));
    reg->add_widget(std::make_shared<Launcher>());
    reg->add_widget(std::make_shared<I3ipc>(loop));

    g_render = std::make_shared<Render>(loop, reg);

    loop.run();

    return 0;
}
