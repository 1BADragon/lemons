#ifndef CPUUSAGE_H
#define CPUUSAGE_H

#include <list>
#include <ev++.h>

#include "widget.h"

class CpuUsage : public Widget
{
public:
    CpuUsage(ev::loop_ref loop);

    void timer_cb(ev::timer &t, int revents);

    virtual std::string name() const override;
    virtual std::string render() const override;

private:
    ev::timer _timer;

    double _last_percent;

    uint64_t _prev_total;
    uint64_t _prev_idle;

    uint64_t _iterval;
};

#endif // CPUUSAGE_H
