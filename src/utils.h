#ifndef UTILS_H
#define UTILS_H

#include <fstream>
#include <string>
#include <sstream>
#include <memory>
#include <unistd.h>
#include <cjson/cJSON.h>

#include "render.h"
#include "colors.h"

std::shared_ptr<Render> render();

static inline std::string fg_color(const std::string &c)
{
    std::stringstream ss;
    ss << "%{F" << c << "}";
    return ss.str();
}

static inline std::string bg_color(const std::string &c)
{
    std::stringstream ss;
    ss << "%{B" << c << "}";
    return ss.str();
}

static inline std::string button(const std::string &content, const std::string &cmd, const char *fg_color, const char *bg_color)
{
    std::stringstream ss;
    ss << ::bg_color(bg_color) << ::fg_color(fg_color)
       << "%{A:" << cmd << ":} " << content << " %{A}"
       << ::bg_color(background) << ::fg_color(foreground);
    return ss.str();
}

static inline ssize_t write_all(int fd, void *_buf, size_t len)
{
    uint8_t *buf = reinterpret_cast<uint8_t *>(_buf);
    size_t total_written = 0;

    while (total_written < len) {
        ssize_t rc = write(fd, buf + total_written, len - total_written);

        if (rc < 0) {
            return -1;
        }
        total_written += rc;
    }

    return total_written;
}

struct cJSON_Deleter
{
    void operator()(cJSON *obj) {
        cJSON_Delete(obj);
    }
};

#endif // UTILS_H
