#ifndef WIDGET_H
#define WIDGET_H

#include <stdexcept>
#include <string>
#include <cstring>
#include <errno.h>
#include <vector>

class Widget {
public:

    class Error : public std::exception
    {
    public:
        Error(const std::string &msg) :
            std::exception(), _msg(msg)
        {

        }

        Error(const std::string &msg, int err_num) :
            std::exception()
        {
            _msg = msg + ": " + strerror(err_num);
        }

        virtual const char *what() const noexcept override
        {
            return _msg.c_str();
        }

    private:
        std::string _msg;
    };

    virtual std::string render() const = 0;
    virtual std::string name() const = 0;

    virtual bool handle_command(const std::string __attribute__((unused)) &cmd) {return true;}
    virtual bool add_commands(std::vector<std::string> &cmds) const {return true;}

};

#endif // WIDGET_H
