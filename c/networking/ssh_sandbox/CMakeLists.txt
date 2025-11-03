cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0135 NEW)
project(SDL3_App C)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(Threads REQUIRED) # required for libssh
find_package(libssh REQUIRED)

add_executable(ssh_sandbox ./source/main.c)
target_include_directories(ssh_sandbox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(ssh_sandbox PRIVATE ssh)

set_target_properties(ssh_sandbox PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
