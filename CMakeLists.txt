cmake_minimum_required(VERSION 3.19)
project(lemons C CXX)

find_library(LIBEV_PATH ev)
find_library(LIBCURL_PATH curl)
find_library(LIBCJSON_PATH cjson)


set(LEMON_SRCS
    src/main.cpp src/render.cpp src/pipe.cpp src/process.cpp src/widgetregistry.cpp
    src/datetime.cpp src/battery.cpp src/diskusage.cpp src/memory.cpp
    src/cpuusage.cpp src/weather.cpp src/power.cpp src/launcher.cpp src/i3ipc.cpp
    )

add_executable(lemons ${LEMON_SRCS})
target_link_libraries(lemons ${LIBEV_PATH} ${LIBCURL_PATH} ${LIBCJSON_PATH})
set_target_properties(lemons
    PROPERTIES
    CXX_STANDARD 17
    C_STANDARD 11)


install(TARGETS lemons)
