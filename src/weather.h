#ifndef WEATHER_H
#define WEATHER_H

#include <ev++.h>
#include "widget.h"

class Weather : public Widget
{
public:
    Weather(ev::loop_ref &loop);

    virtual std::string name() const override;
    virtual std::string render() const override;

    void timer_cb(ev::timer &t, int revents);

private:
    ev::loop_ref _loop;
    ev::timer _timer_watcher;

    double _temp;
    std::string _icon;
};

#endif // WEATHER_H
