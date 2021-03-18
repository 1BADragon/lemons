#ifndef I3IPC_H
#define I3IPC_H

#include <string>
#include <list>
#include <ev++.h>
#include <i3/ipc.h>

#include "widget.h"

class I3ipc : public Widget
{
public:
    I3ipc(ev::loop_ref &loop);
    virtual ~I3ipc();

    virtual std::string name() const override;
    virtual std::string render() const override;
    virtual bool add_commands(std::vector<std::string> &cmds) const override;
    virtual bool handle_command(const std::string &cmd) override;

private:
    ev::loop_ref _loop;
    ev::io _io_watcher;
    ev::timer _disconnected_timer;
    std::string _i3sock_path;

    enum {
        READ_HEADER,
        READ_BODY
    } _state;

    i3_ipc_header _hdr;
    std::string _body;
    size_t _read_at;

    void ipc_cb(ev::io &i, int revents);
    void timer_cb(ev::timer &i, int revents);

    int open_ipc_socket();
    void reconnect_socket();

    void subscribe();
    void get_workspaces();

    void handle_ipc_msg();
    void workspace_msg();

    void handle_event_msg();
    void workspace_event();

    void subscribe_msg();

    void sort_workspaces();
    void send_i3_cmd(const std::string &cmd);

    struct Workspace {
        uint32_t num;
        std::string name;
        bool active;
    };

    std::list<Workspace> _workspaces;

    static bool workspace_less(const Workspace &a, const Workspace &b);
};

#endif // I3IPC_H
