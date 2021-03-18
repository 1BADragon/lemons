#include <cstdlib>
#include <sstream>
#include <unistd.h>
#include <algorithm>
#include <fcntl.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <i3/ipc.h>

#include "i3ipc.h"
#include "utils.h"

static const char *i3ipc_env = "I3SOCK";

I3ipc::I3ipc(ev::loop_ref &loop) :
    _loop(loop)
{
    _i3sock_path = secure_getenv(i3ipc_env);

    _state = READ_HEADER;
    _read_at = 0;

    reconnect_socket();
}

I3ipc::~I3ipc()
{
    _io_watcher.stop();
    close(_io_watcher.fd);
}

std::string I3ipc::name() const
{
    return "i3ipc";
}

std::string I3ipc::render() const
{

    if (_io_watcher.fd == -1) {
        return "I3 disconnected";
    }

    std::stringstream ss;

    for (auto &w : _workspaces) {
        if (w.active) {
            ss << button(std::to_string(w.num), "ws-" + std::to_string(w.num), dark0_hard, bright_green);
        } else {
            ss << button(std::to_string(w.num), "ws-" + std::to_string(w.num), dark0_hard, foreground);
        }
        ss << " ";
    }

    return ss.str();
}

bool I3ipc::add_commands(std::vector<std::string> &cmds) const
{
    cmds.push_back("ws");

    return true;
}

bool I3ipc::handle_command(const std::string &cmd)
{
    std::string i3_cmd = "workspace number ";

    i3_cmd += cmd.substr(cmd.find("-") + 1);

    send_i3_cmd(i3_cmd);

    return true;
}

void I3ipc::ipc_cb(ev::io &i, int revents)
{
    while (true) {
        int rc;
        switch (_state) {
        case READ_HEADER:
            rc = read(i.fd, reinterpret_cast<char *>(&_hdr) + _read_at,
                      sizeof(i3_ipc_header) - _read_at);

            if (rc == 0) {
                reconnect_socket();
                ::render()->update();
                return;
            }

            if (rc < 0) {
                if (errno == EAGAIN) {
                    return;
                }
                throw Error("Failed reading from IPC", errno);
            }
            _read_at += rc;

            if (_read_at == sizeof(i3_ipc_header)) {
                _read_at = 0;
                _state = READ_BODY;
                _body.resize(_hdr.size);
                if (strncmp(I3_IPC_MAGIC, _hdr.magic, 6)) {
                    throw Error(std::string("Bad i3 magic value: ") +
                                std::string(_hdr.magic, 6));
                }
            }
            break;
        case READ_BODY:
            rc = read(i.fd, _body.data() + _read_at, _body.size() - _read_at);

            if (rc == 0) {
                reconnect_socket();
                ::render()->update();
                return;
            }

            if (rc < 0) {
                if (errno == EAGAIN) {
                    return;
                }
                throw Error("Failed reading from IPC", errno);
            }
            _read_at += rc;

            if (_read_at == _body.size()) {
                _read_at = 0;
                _state = READ_HEADER;
                handle_ipc_msg();
                ::render()->update();
            }
            break;
        }
    }
}

void I3ipc::timer_cb(ev::timer &i, int revents)
{
    int fd = open_ipc_socket();

    if (fd == -1) {
        return;
    }

    _disconnected_timer.stop();

    get_workspaces();

    _io_watcher.set(fd, ev::READ);
    _io_watcher.set<I3ipc, &I3ipc::ipc_cb>(this);
    _io_watcher.set(_loop);
    _io_watcher.start();

    subscribe();
    get_workspaces();
    ::render()->update();
}

int I3ipc::open_ipc_socket()
{
    struct sockaddr_un addr;
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (-1 == fd) {
        throw Error("Unable to open ipc socket", errno);
    }

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, _i3sock_path.c_str(), sizeof(addr.sun_path) - 1);

    if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(struct sockaddr_un))) {
        close(fd);
        return -1;
    }

    int flags = fcntl(fd, F_GETFL, NULL);

    flags |= O_NONBLOCK;

    fcntl(fd, F_SETFL, flags);

    return fd;
}

void I3ipc::reconnect_socket()
{
    _io_watcher.stop();
    close(_io_watcher.fd);
    _io_watcher.fd = -1;

    _disconnected_timer.set(0.0, 1.0);
    _disconnected_timer.set<I3ipc, &I3ipc::timer_cb>(this);
    _disconnected_timer.set(_loop);
    _disconnected_timer.start();
}

void I3ipc::subscribe()
{
    std::string payload(sizeof(i3_ipc_header), '\0');
    i3_ipc_header *hdr = reinterpret_cast<i3_ipc_header *>(payload.data());

    strncpy(hdr->magic, I3_IPC_MAGIC, sizeof(hdr->magic));
    hdr->type = I3_IPC_MESSAGE_TYPE_SUBSCRIBE;

    std::string message = "[ \"workspace\" ]";
    hdr->size = message.size();

    payload += message;
    write(_io_watcher.fd, payload.data(), payload.size());
}

void I3ipc::get_workspaces()
{
    i3_ipc_header hdr;

    strncpy(hdr.magic, I3_IPC_MAGIC, sizeof(hdr.magic));
    hdr.type = I3_IPC_MESSAGE_TYPE_GET_WORKSPACES;
    hdr.size = 0;

    write_all(_io_watcher.fd, &hdr, sizeof(i3_ipc_header));
}

void I3ipc::handle_ipc_msg()
{
    if (_hdr.type & I3_IPC_EVENT_MASK) {
        handle_event_msg();
    }

    switch (_hdr.type) {
    case I3_IPC_REPLY_TYPE_WORKSPACES:
        workspace_msg();
        break;
    default:
        break;
    }
}

void I3ipc::workspace_msg()
{
    std::unique_ptr<cJSON, cJSON_Deleter> body(cJSON_Parse(_body.c_str()));
    cJSON *at;

    _workspaces.clear();

    cJSON_ArrayForEach(at, body.get()) {
        std::string name(cJSON_GetStringValue(cJSON_GetObjectItem(at, "name")));
        bool visible = cJSON_IsTrue(cJSON_GetObjectItem(at, "visible"));
        uint32_t num = cJSON_GetNumberValue(cJSON_GetObjectItem(at, "num"));

        _workspaces.push_back({num, name, visible});
    }

    sort_workspaces();
}

void I3ipc::handle_event_msg()
{
    switch (_hdr.type) {
    case I3_IPC_EVENT_WORKSPACE:
        workspace_event();
        break;
    }
}

void I3ipc::workspace_event()
{
    std::unique_ptr<cJSON, cJSON_Deleter> body(cJSON_Parse(_body.c_str()));

    std::string change(cJSON_GetStringValue(cJSON_GetObjectItem(body.get(), "change")));

    if (change == "focus") {
        uint32_t old_num = cJSON_GetNumberValue(cJSON_GetObjectItem(
                                                    cJSON_GetObjectItem(body.get(), "old"), "num"));
        uint32_t new_num = cJSON_GetNumberValue(cJSON_GetObjectItem(
                                                    cJSON_GetObjectItem(body.get(), "current"), "num"));
        for (auto &w : _workspaces) {
            if (w.num == old_num) {
                w.active = false;
            }

            if (w.num == new_num) {
                w.active = true;
            }
        }

    } else if (change == "init") {
        auto curr = cJSON_GetObjectItem(body.get(), "current");

        std::string name(cJSON_GetStringValue(cJSON_GetObjectItem(curr, "name")));
        uint32_t num = cJSON_GetNumberValue(cJSON_GetObjectItem(curr, "num"));

        for (auto &w: _workspaces) {
            w.active = false;
        }
        _workspaces.push_back({num, name, true});
    } else if (change == "empty") {
        auto curr = cJSON_GetObjectItem(body.get(), "current");
        uint32_t num = cJSON_GetNumberValue(cJSON_GetObjectItem(curr, "num"));

        for (auto it = _workspaces.begin(); it != _workspaces.end(); ) {
            if (it->num == num) {
                it = _workspaces.erase(it);
            } else {
                ++it;
            }
        }
    }

    sort_workspaces();
    ::render()->update();
}

void I3ipc::subscribe_msg()
{

}

void I3ipc::sort_workspaces()
{
    _workspaces.sort(&I3ipc::workspace_less);
}

void I3ipc::send_i3_cmd(const std::string &cmd)
{
    i3_ipc_header *msg = reinterpret_cast<i3_ipc_header*>(alloca(sizeof(i3_ipc_header) + cmd.size()));

    memcpy(&msg->magic, I3_IPC_MAGIC, sizeof(msg->magic));
    msg->size = cmd.size();
    msg->type = I3_IPC_MESSAGE_TYPE_COMMAND;
    memcpy(&(msg[1]), cmd.data(), cmd.size());

    write_all(_io_watcher.fd, msg, sizeof(i3_ipc_header) + cmd.size());
}

bool I3ipc::workspace_less(const I3ipc::Workspace &a, const I3ipc::Workspace &b)
{
    return a.num < b.num;
}


