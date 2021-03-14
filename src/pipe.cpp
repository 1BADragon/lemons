#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <iostream>

#include "pipe.h"

Pipe::PipeError::PipeError(const std::string &_val) :
    std::exception(), _msg(_val)
{
}

const char * Pipe::PipeError::what() const noexcept
{
    return _msg.c_str();
}

Pipe::Pipe(const ev::loop_ref &loop) :
    _loop(loop)
{
    int fds[2];
    int rc;

    rc = pipe(fds);

    if (rc) {
        const char * errmsg = strerror(errno);
        throw PipeError(std::string("Unable to construct pipe: ") + errmsg);
    }

    _read_fd = fds[0];
    _write_fd = fds[1];
    _mode = Mode::BOTH;

    init_watchers();
}

Pipe::Pipe(int reader, int writer, const ev::loop_ref &loop,  Mode m) :
    _loop(loop), _write_fd(writer), _read_fd(reader), _mode(m)
{
    init_watchers();
}

Pipe::Pipe(const Pipe &other) :
    _loop(other._loop), _write_fd(dup(other.write_fileno())), _read_fd(dup(other.read_fileno())), _mode(other._mode)
{
    init_watchers();
}

Pipe::~Pipe()
{
    _writeable_watcher.stop();
    _readable_watcher.stop();

    ::close(_write_fd);
    ::close(_read_fd);
}

lem::Bytes Pipe::read(size_t amount)
{
    if (!(_mode & Mode::READ)) {
        throw PipeError("Pipe is write only");
    }

    if (amount == 0) {
        auto ret = _read_buffer.str();
        _read_buffer = lem::ByteStream();
        return ret;
    } else {
        lem::Bytes ret;
        ret.resize(amount);

        size_t read_ret = _read_buffer.readsome(ret.data(), amount);
        ret.resize(read_ret);
        return ret;
    }
}

void Pipe::write(const lem::Bytes &data)
{
    if (!(_mode & Mode::WRITE)) {
        throw PipeError("Pipe is read only");
    }

    _write_buffer << data;
    _writeable_watcher.start();
}

void Pipe::close(Mode m)
{
    if (m == Mode::READ) {
        ::close(_read_fd);
        _readable_watcher.stop();
        _read_fd = -1;
    }

    if (m == Mode::WRITE) {
        ::close(_write_fd);
        _writeable_watcher.stop();
        _write_fd = -1;
    }

    _mode &= ~m;
}

int Pipe::write_fileno() const noexcept
{
    return _write_fd;
}

int Pipe::read_fileno() const noexcept
{
    return _read_fd;
}

void Pipe::read_cb(ev::io &watcher, int revents)
{
    if (revents & ev::READ) {
        lem::Bytes buf;
        buf.resize(256);
        int rc = ::read(_read_fd, buf.data(), buf.size());

        //fprintf(stderr, "Read %d bytes", rc);

        if (rc < 0) {
            throw PipeError(std::string("IO failure: ") + strerror(errno));
        }

        buf.resize(rc);

        _read_buffer << buf;

        if (io_cb) {
            io_cb(this, ev::READ);
        }
    } else {
        throw PipeError("Recieved invalid event: " + std::to_string(revents));
    }
}

void Pipe::write_cb(ev::io &watcher, int revents)
{
    if (revents & ev::WRITE) {
        lem::Bytes buf;
        buf.resize(256);
        int rc1 = _write_buffer.readsome(buf.data(), buf.size());

        if (rc1 == 0) {
            // The write buffer is empty
            _writeable_watcher.stop();
            return;
        }

        int rc2 = ::write(_write_fd, buf.data(), rc1);

        //fprintf(stderr, "Write %d bytes", rc2);

        if (rc2 < 0) {
            throw PipeError(std::string("IO failure") + strerror(errno));
        }

        if (rc2 < rc1) {
            // I didn't write the whole buffer;
            _write_buffer = lem::ByteStream(buf.substr(rc2) + _write_buffer.str());
        }
    } else {
        throw PipeError("Recieved invalid event: " + std::to_string(revents));
    }
}

void Pipe::init_watchers()
{
    if (_mode & Mode::WRITE) {
        _writeable_watcher.set(_write_fd, ev::WRITE);
        _writeable_watcher.set(_loop);
        _writeable_watcher.set<Pipe, &Pipe::write_cb>(this);
    }

    if (_mode & Mode::READ) {
        _readable_watcher.set(_read_fd, ev::READ);
        _readable_watcher.set(_loop);
        _readable_watcher.set<Pipe, &Pipe::read_cb>(this);
        _readable_watcher.start();
    }
}

