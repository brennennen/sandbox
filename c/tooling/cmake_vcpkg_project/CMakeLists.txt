
cmake_minimum_required(VERSION 4.1.2)
project(sdr LANGUAGES C)

find_package(sdl3 CONFIG REQUIRED)

add_executable(main ./source/main.c)
target_link_libraries(main PRIVATE SDL3::SDL3)
