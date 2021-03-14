#ifndef MEMORY_H
#define MEMORY_H

#include <ev++.h>

#include "widget.h"

class Memory : public Widget
{
public:
    Memory(ev::loop_ref &loop);

    void timer_cb(ev::timer &t, int revents);

    virtual std::string name() const override;
    virtual std::string render() const override;

private:
    ev::timer _timer;
    double _last_percent;
};

#endif // MEMORY_H
