cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0135 NEW)
project(SDL3_App C)
set(VENDOR_DIR "${CMAKE_SOURCE_DIR}/.vendor")
include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/.vendor/_deps")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(sdl3)
include(sdl3_ttf)
include(sdl3_image)
include(clay)

add_executable(main main.c)
target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(main PRIVATE 
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    SDL3_image::SDL3_image
    clay::clay
)

if(WIN32)
    # Copy DLLs over to main build directory on windows
    add_custom_command(
        TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL3_DLL_PATH}"
        $<TARGET_FILE_DIR:main>
        COMMENT "Copying SDL3.dll to build directory"
    )
    add_custom_command(
        TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL3_TTF_DLL_PATH}"
        $<TARGET_FILE_DIR:main>
        COMMENT "Copying SDL3_ttf.dll to build directory"
    )
    add_custom_command(
        TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL3_IMAGE_DLL_PATH}"
        $<TARGET_FILE_DIR:main>
        COMMENT "Copying SDL3_image.dll to build directory"
    )
endif()

# copy resources to output binary location
add_custom_command(
    TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "./../../../../../resources/fonts/Roboto-Regular.ttf"
    "$<TARGET_FILE_DIR:main>/resources/Roboto-Regular.ttf"
    COMMENT "Copying font to build directory"
)
add_custom_command(
    TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "./../../../../../resources/images/sample.png"
    "$<TARGET_FILE_DIR:main>/resources/sample.png"
    COMMENT "Copying font to build directory"
)

set_target_properties(main PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
