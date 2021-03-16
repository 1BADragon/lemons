#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <signal.h>

#include "process.h"

Process::ProcessError::ProcessError(const std::string &s) :
    _msg(s)
{

}

const char * Process::ProcessError::what() const noexcept
{
    return _msg.c_str();
}

Process::Process(const ev::loop_ref &loop) :
    _path(), _args(), _io_cb(), _pid(-1), _child_out(loop), _child_in(loop),
    _child_watcher(loop), _loop(loop), _state(INIT)
{

}

Process::Process(const ev::loop_ref &loop, const std::string &path, const std::vector<std::string> &args) :
    _path(path), _args(args), _io_cb(), _pid(-1), _child_out(loop), _child_in(loop),
    _child_watcher(loop), _loop(loop), _state(INIT)
{

}

Process::~Process()
{
    if (_pid != -1) {
        kill();
    }
}

void Process::set_path(const std::string &path)
{
    if (_path.size() > 0) {
        throw ProcessError("Program path already set");
    }
    _path = path;
}

void Process::add_arg(const std::string &arg)
{
    _args.push_back(arg);
}

Process& Process::operator<<(const std::string &arg)
{
    add_arg(arg);
    return *this;
}

void Process::operator()(Pipe *p, int revent)
{
    if (_io_cb) {
        _io_cb(this, revent);
    }
}

void Process::run()
{
    if (_pid != -1) {
        throw ProcessError("Cannot lauch already running process");
    }

    int child_pid = fork();

    if (child_pid == 0) {
        prctl(PR_SET_PDEATHSIG, SIGKILL);

        // child process
        dup2(_child_in.read_fileno(), STDIN_FILENO);
        dup2(_child_out.write_fileno(), STDOUT_FILENO);
        dup2(_child_out.write_fileno(), STDERR_FILENO);

        _child_in.close();
        _child_out.close();

        std::vector<const char *> args;

        args.push_back(_path.c_str());
        for (auto &a : _args) {
            args.push_back(a.c_str());
        }
        args.push_back(nullptr);

        execvp(_path.c_str(), const_cast<char* const *>(args.data()));

        _exit(-127);
    } else {
        _pid = child_pid;

        _child_in.set_cb(this);
        _child_out.set_cb(this);

        _child_in.close(Pipe::READ);
        _child_out.close(Pipe::WRITE);

        _child_watcher.set<Process, &Process::child_cb>(this);
        _child_watcher.set(_pid);
        _child_watcher.start();

        _state = RUNNING;
    }
}

int Process::exec(const ev::loop_ref &loop, const std::string &path, const std::vector<std::string> &args)
{
    Process p(loop, path, args);
    p.run();
    return p.wait_for_exit();
}

lem::Bytes Process::read(size_t amount)
{
    if (_pid == -1) {
        throw ProcessError("No program running");
    }
    return _child_out.read(amount);
}

void Process::write(const lem::Bytes &data)
{
    if (_pid == -1) {
        throw ProcessError("No program running");
    }
    return _child_in.write(data);
}

void Process::signal(int sig_num)
{
    if (_pid == -1) {
        throw ProcessError("No program running");
    }

    if (::kill(_pid, sig_num)) {
        throw ProcessError(std::string("Unable to send signal: ") + strerror(errno));
    }
}

void Process::kill()
{
    ::kill(_pid, SIGKILL);
    waitpid(_pid, nullptr, 0);
}

Process::State Process::state() const
{
    return _state;
}

int Process::wait_for_exit()
{
    if (_state != RUNNING) {
        throw ProcessError("Can only wait on running process");
    }

    _state = WAITING;

    while (_state != STOPPED) {
        _loop.run();
    }

    return _exit_code;
}

int Process::exit_code() const
{
    return _exit_code;
}

void Process::child_cb(ev::child &c, int revents)
{
    if (_state == WAITING) {
        _loop.break_loop();
    }

    _state = STOPPED;
    _exit_code = c.rstatus;

    if (_io_cb) {
        _io_cb(this, revents);
    }
}
