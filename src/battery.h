#ifndef BATTERY_H
#define BATTERY_H

#include <ev++.h>

#include "widget.h"

class Battery : public Widget
{
public:
    Battery(ev::loop_ref &loop);

    virtual std::string name() const override;
    virtual std::string render() const override;

    void timer_cb(ev::timer &t, int revent);

private:
    ev::timer _timer;

    double _last_percent;
    std::string _last_status;
};

#endif // BATTERY_H
