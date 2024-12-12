cmake_minimum_required (VERSION 3.0)
project(cmake_testing)

set(testing_enabled "False")

# Default builds to release.
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "release")
endif()

# Shared settings
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

# Handle release vs debug specific settings
if(CMAKE_BUILD_TYPE STREQUAL "release")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_DEBUG} -wall")
    message("Release flags: ${CMAKE_C_FLAGS} {CMAKE_C_FLAGS_RELEASE}")
elseif(CMAKE_BUILD_TYPE STREQUAL "debug")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb")
    enable_testing()
    message("Debug flags: ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")
else()
    message(FATAL_ERROR "Please pass in CMAKE_BUILD_TYPE as either 'release' or 'debug'. Example: cmake .. -DCMAKE_BUILD_TYPE=release")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin)
message("Output binary directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message("Output library directory: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

add_subdirectory(libraries/lib_a)
add_subdirectory(libraries/lib_b)
add_subdirectory(libraries/lib_c)
add_subdirectory(libraries/lib_d)
add_subdirectory(libraries/lib_e)

add_subdirectory(modules/module_a)
add_subdirectory(modules/module_b)
add_subdirectory(modules/module_c)
