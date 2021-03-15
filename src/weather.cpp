#include <iostream>
#include <sstream>
#include <iomanip>
#include <cjson/cJSON.h>
#include <unordered_map>

#include "process.h"
#include "weather.h"
#include "utils.h"
#include "colors.h"

static const char * url = "https://api.weather.gov/stations/kmry/observations/latest";
static const char *sunny = "\uf185";
static const char *cloud = "\uf0c2";
static const char *wind  = "\uf72e";
static const char *rain_cloud_sun = "\uf743";
static const char *snow = "\uf2dc";
static const char *cloud_sun = "\uf6c4";
static const char *cloud_rain = "\uf73d";
static const char *smog = "\uf75f";
static const char *cloud_shower_heavy = "\uf740";

// This will grow as new weather types are discovered
std::unordered_map<std::string, const char *> icons = {
    {"Mostly Cloudy", cloud_sun},
    {"Cloudy", cloud},
    {"Light Rain", cloud_rain},
};

static double c_to_f(double c);

Weather::Weather(ev::loop_ref &loop) :
    _loop(loop), _timer_watcher(loop)
{
    _timer_watcher.set<Weather, &Weather::timer_cb>(this);

    _timer_watcher.start(0, 3600); // hourly
}

std::string Weather::name() const
{
    return "weather";
}

std::string Weather::render() const
{
    std::stringstream ss;

    ss << _icon << " " << std::fixed << std::setprecision(1)
       << fg_color(bright_green) << _temp;

    return ss.str();
}

void Weather::timer_cb(ev::timer &t, int revents)
{
    Process p(_loop);

    p.set_path("curl");
    p << "curl" << "-s" << url;
    p.run();

    p.wait_for_exit();
    auto data = p.read();

    cJSON *obj = cJSON_ParseWithLength(reinterpret_cast<char*>(data.data()),
                                       data.length());

    if (nullptr == obj) {
        throw Widget::Error("Failed to parse weather output");
    }

    auto props = cJSON_GetObjectItem(obj, "properties");
    auto temp = cJSON_GetObjectItem(props, "temperature");
    auto deg = cJSON_GetNumberValue(cJSON_GetObjectItem(temp, "value"));
    auto dis = std::string(cJSON_GetStringValue(cJSON_GetObjectItem(props, "textDescription")));

    _temp = c_to_f(deg);

    auto it = icons.find(dis);

    if (it != icons.end()) {
        _icon = it->second;
    } else {
        _icon = dis;
    }

    cJSON_Delete(obj);
}

static double c_to_f(double c)
{
    return (c * 9./5.) + 32.;
}
