#ifndef LAUNCHER_H
#define LAUNCHER_H

#include "widget.h"

class Launcher : public Widget
{
public:
    Launcher();

    virtual std::string name() const override;
    virtual std::string render() const override;
};

#endif // LAUNCHER_H
