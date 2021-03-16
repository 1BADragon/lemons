#ifndef POWER_H
#define POWER_H

#include <ev++.h>
#include "widget.h"

class Power : public Widget
{
public:

    class Dialog : public Widget
    {
    public:
        Dialog(ev::loop_ref &loop);

        virtual std::string name() const override;
        virtual std::string render() const override;
        virtual bool handle_command(const std::string &cmd) override;

    private:
        ev::timer _timer;
        ev::loop_ref _loop;

        void timer_cb(ev::timer &t, int revents);
    };

    Power(ev::loop_ref &loop);

    virtual std::string name() const override;
    virtual std::string render() const override;
    virtual bool add_commands(std::vector<std::string> &cmds) const override;
    virtual bool handle_command(const std::string &cmd) override;

private:
    ev::loop_ref _loop;
};

#endif // POWER_H
