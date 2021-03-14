#ifndef DISKUSAGE_H
#define DISKUSAGE_H

#include <string>
#include <ev++.h>

#include "widget.h"

class DiskUsage : public Widget
{
public:
    DiskUsage(ev::loop_ref loop, const std::string &path = "/");

    void timer_cb(ev::timer &t, int revents);

    virtual std::string render() const override;
    virtual std::string name() const override;

private:
    std::string _path;
    double _last_percent;

    ev::timer _timer;
};

#endif // DISKUSAGE_H
