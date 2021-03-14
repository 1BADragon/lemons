#include <iostream>
#include <functional>
#include <memory>

#include <ev++.h>

#include "widgetregistry.h"
#include "render.h"
#include "datetime.h"
#include "battery.h"
#include "diskusage.h"
#include "memory.h"
#include "cpuusage.h"

struct Interrupt {
    void operator()(ev::sig &sig, int event)
    {
        sig.loop.break_loop(ev::ALL);
    }
};

int main()
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

    auto render = std::make_shared<Render>(loop, reg);

    loop.run();
    return 0;
}
