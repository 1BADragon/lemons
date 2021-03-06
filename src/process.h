#ifndef PROCESS_H
#define PROCESS_H

#include <stdexcept>
#include <string>
#include <vector>
#include <ev++.h>

#include "pipe.h"
#include "types.h"

class Process
{
public:
    class ProcessError : public std::exception
    {
    public:
        ProcessError(const std::string &s);

        virtual const char *what() const noexcept override;

    private:
        std::string _msg;
    };

    enum State {
        INIT,
        RUNNING,
        WAITING,
        STOPPED
    };

    Process(const ev::loop_ref &loop);
    Process(const ev::loop_ref &loop, const std::string &path, const std::vector<std::string> &args = {});

    virtual ~Process();

    void set_path(const std::string &path);
    void add_arg(const std::string &arg);

    Process& operator<<(const std::string &arg);
    void operator()(Pipe *p, int revent);

    void run();
    static int exec(const ev::loop_ref &loop, const std::string &path,
                    const std::vector<std::string> &args = {});

    lem::Bytes read(size_t amount = 0);
    void write(const lem::Bytes &data);

    void signal(int sig_num);

    // Also waits for process to exit
    void kill();

    template<typename T>
    void set_iocb(T* object)
    {
        if (object == NULL) {
            _io_cb = nullptr;
            return;
        }

        _io_cb = std::bind(static_cast<void (T::*)(Process*, int)>(&T::operator()),
                           object, std::placeholders::_1, std::placeholders::_2);
    }

    State state() const;
    int wait_for_exit();
    int exit_code() const;

private:
    std::string _path;
    std::vector<std::string> _args;

    std::function<void(Process *, int)> _io_cb;

    int _pid;
    Pipe _child_out;
    Pipe _child_in;
    ev::child _child_watcher;
    ev::loop_ref _loop;
    State _state;
    int _exit_code;

    void child_cb(ev::child &c, int revents);
};

#endif // PROCESS_H
