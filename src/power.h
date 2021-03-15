#ifndef POWER_H
#define POWER_H

#include "widget.h"

class Power : public Widget
{
public:
    Power();

    virtual std::string name() const override;
    virtual std::string render() const override;
    virtual bool add_commands(std::vector<std::string> &cmds) const override;
    virtual bool handle_command(const std::string &cmd) override;
};

#endif // POWER_H
