#ifndef DATETIME_H
#define DATETIME_H

#include "widget.h"

class DateTime : public Widget
{
public:
    virtual std::string render() const override;
    virtual std::string name() const override;
};

#endif // DATETIME_H
