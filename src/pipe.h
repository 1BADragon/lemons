#ifndef PIPE_H
#define PIPE_H

#include <string>
#include <stdexcept>
#include <ev++.h>
#include <functional>

#include "types.h"

class Pipe {
public:
    class PipeError : public std::exception {
    public:
        PipeError(const std::string &_val);
        virtual const char *what() const noexcept override;
    private:
        std::string _msg;
    };

    enum Mode {
        NONE = 0,
        READ = 0x1,
        WRITE = 0x2,
        BOTH = READ | WRITE
    };

    Pipe(const ev::loop_ref &loop);
    Pipe(int reader, int writer, const ev::loop_ref &loop, Mode m = Mode::BOTH);
    Pipe(const Pipe &other);
    virtual ~Pipe();

    lem::Bytes read(size_t amount = 0);
    void write(const lem::Bytes &data);

    void close(Mode m = Mode::BOTH);

    int write_fileno() const noexcept;
    int read_fileno() const noexcept;

    void read_cb(ev::io &watcher, int revents);
    void write_cb(ev::io &watcher, int revents);

    template<typename T>
    void set_cb(T *obj)
    {
        if (obj == nullptr) {
            io_cb = nullptr;
            return;
        }

        io_cb = std::bind(static_cast<void(T::*)(Pipe*, int)>(&T::operator()),
                          obj, std::placeholders::_1, std::placeholders::_2);
    }

private:
    lem::ByteStream _read_buffer;
    lem::ByteStream _write_buffer;

    ev::io _writeable_watcher;
    ev::io _readable_watcher;
    ev::loop_ref _loop;

    int _write_fd;
    int _read_fd;
    int _mode;

    void init_watchers();
    std::function<void (Pipe *, int)> io_cb;
};

#endif // PIPE_H
