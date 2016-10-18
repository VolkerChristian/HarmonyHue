cmake_minimum_required(VERSION 2.6)
set(CMAKE_TOOLCHAIN_FILE /home/voc/usr/toolchains/armhf/armhf.cmake)

project(harmonyhueconnect)

add_executable(harmonyhueconnect HarmonyHubControl.cpp csocket.cpp csocket.h)

install(TARGETS harmonyhueconnect RUNTIME DESTINATION bin)
