
set(SDL3_VERSION "3.2.24" CACHE STRING "SDL3 version to download")
if(NOT VENDOR_DIR)
    set(VENDOR_DIR "${CMAKE_SOURCE_DIR}/.vendor" CACHE PATH "Directory for downloaded libraries")
endif()
message(STATUS "Using vendor directory: ${VENDOR_DIR}")

include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${VENDOR_DIR}/_deps CACHE PATH "Persistent location for downloaded dependencies")

if(NOT TARGET SDL3::SDL3)
    message(STATUS "Attempting to find or build SDL3...")
    # For windows, download pre-compiled binaries
    if(WIN32)
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
        
        # Create an INTERFACE library to represent the downloaded SDL3
        add_library(SDL3::SDL3 INTERFACE IMPORTED GLOBAL)
        target_include_directories(SDL3::SDL3 INTERFACE "${SDL3_DIST_DIR}/include")
        target_link_libraries(SDL3::SDL3 INTERFACE "${SDL3_DIST_DIR}/lib/libSDL3.dll.a")

        set(SDL3_DLL_PATH "${SDL3_DIST_DIR}/bin/SDL3.dll" CACHE FILEPATH "Path to SDL3.dll")
    # For linux, build from source
    elseif(UNIX AND NOT APPLE)
        find_package(PkgConfig QUIET)
        if (PKG_CONFIG_FOUND)
            pkg_check_modules(PC_SDL3 QUIET sdl3)
        endif()

        if (PC_SDL3_FOUND)
            message(STATUS "Found system-wide SDL3 via pkg-config")
            find_package(SDL3 CONFIG REQUIRED)
        else()
            message(STATUS "SDL3 not found via pkg-config. Building from source...")
            include(FetchContent)
            FetchContent_Declare(
                sdl3_source
                GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
                GIT_TAG        release-${SDL3_VERSION}
            )
            FetchContent_MakeAvailable(sdl3_source)
        endif()
    else()
        message(FATAL_ERROR "Unsupported platform for automatic SDL3 setup.")
    endif()
endif()

# --- Final Status ---
if(TARGET SDL3::SDL3)
    message(STATUS "SDL3 setup complete. Use SDL3::SDL3 target for linking.")
endif()
