cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0135 NEW) # FetchContent timestamp behavior warning
project(sdl3_hello_world C)
include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/.vendor/_deps")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(sdl3)
add_executable(main main.c)
target_link_libraries(main PRIVATE SDL3::SDL3)

if(WIN32)
    # Copy DLLs over to main build directory on windows
    add_custom_command(
        TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL3_DLL_PATH}"
        $<TARGET_FILE_DIR:main>
        COMMENT "Copying SDL3.dll to build directory"
    )
endif()
set_target_properties(main PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
