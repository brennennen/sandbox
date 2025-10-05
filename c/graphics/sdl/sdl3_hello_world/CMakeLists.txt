cmake_minimum_required(VERSION 3.15)
project(sdl3_hello_world C)
set(VENDOR_DIR "${CMAKE_SOURCE_DIR}/.vendor" CACHE PATH "Directory for downloaded libraries" FORCE)

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
message(STATUS "SDL3 Include Dirs: ${SDL3_INCLUDE_DIRS}")
message(STATUS "SDL3 Libraries: ${SDL3_LIBRARIES}")
message(STATUS "Project configured. You can now build it with 'cmake --build .'")
