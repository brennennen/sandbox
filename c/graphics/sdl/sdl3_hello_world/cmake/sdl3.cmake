
if(NOT TARGET SDL3::SDL3)
    message(STATUS "Attempting to find or build SDL3...")

    # For windows, download pre-compiled binaries
    if(WIN32)
        set(SDL3_VERSION "3.2.24")
        set(SDL3_ARCHIVE_NAME "SDL3-devel-${SDL3_VERSION}-mingw.zip")
        set(SDL3_URL "https://github.com/libsdl-org/SDL/releases/download/release-${SDL3_VERSION}/${SDL3_ARCHIVE_NAME}")
        FetchContent_Declare(
            sdl3_binary
            URL          ${SDL3_URL}
            # cmake -E sha256sum SDL3-devel-3.2.24-mingw.zip
            URL_HASH     SHA256=9e63851baf375d4453cc3707b32ccb6bf1891011efd7c214c1ea6ebbe6c936b0
        )
        FetchContent_MakeAvailable(sdl3_binary)
        set(SDL3_DIST_DIR "${sdl3_binary_SOURCE_DIR}/x86_64-w64-mingw32")

        add_library(SDL3::SDL3 INTERFACE IMPORTED GLOBAL)
        target_include_directories(SDL3::SDL3 INTERFACE "${SDL3_DIST_DIR}/include")
        target_link_libraries(SDL3::SDL3 INTERFACE "${SDL3_DIST_DIR}/lib/libSDL3.dll.a")

        set(SDL3_DLL_PATH "${SDL3_DIST_DIR}/bin/SDL3.dll")

    # For linux, use find_package
    elseif(UNIX AND NOT APPLE)
        find_package(SDL3 CONFIG REQUIRED)
    else()
        message(FATAL_ERROR "Unsupported platform for automatic SDL3 setup.")
    endif()
endif()

if(TARGET SDL3::SDL3)
    message(STATUS "SDL3 setup complete. Use SDL3::SDL3 target for linking.")
endif()
