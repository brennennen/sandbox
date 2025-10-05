
set(SDL3_IMAGE_VERSION "3.2.4")

if(NOT TARGET SDL3_IMAGE::SDL3_IMAGE)
    message(STATUS "Attempting to find or build SDL3_image...")
    # For windows, download pre-compiled binaries and use those
    if(WIN32)
        set(SDL3_IMAGE_ARCHIVE_NAME "SDL3_image-devel-${SDL3_IMAGE_VERSION}-mingw.zip")
        # https://github.com/libsdl-org/SDL_image/releases/download/release-3.2.4/SDL3_image-devel-3.2.4-mingw.zip
        set(SDL3_IMAGE_URL "https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL3_IMAGE_VERSION}/${SDL3_IMAGE_ARCHIVE_NAME}")
        FetchContent_Declare(
            sdl3_image_binaries
            URL          ${SDL3_IMAGE_URL}
            # cmake -E sha256sum SDL3_image-devel-3.2.4-mingw.zip
            URL_HASH     SHA256=e3e77d3136b6ba8d8808af1920cdfc36a211765fd9ad2a1bb81718fd56d4a5ee
        )
        FetchContent_MakeAvailable(sdl3_image_binaries)
        set(SDL3_IMAGE_DIST_DIR "${sdl3_image_binaries_SOURCE_DIR}/x86_64-w64-mingw32")
        add_library(SDL3_image::SDL3_image INTERFACE IMPORTED)
        target_include_directories(SDL3_image::SDL3_image INTERFACE "${SDL3_IMAGE_DIST_DIR}/include")
        target_link_libraries(SDL3_image::SDL3_image INTERFACE "${SDL3_IMAGE_DIST_DIR}/lib/libSDL3_image.dll.a")
        set(SDL3_IMAGE_DLL_PATH "${SDL3_IMAGE_DIST_DIR}/bin/SDL3_image.dll")

    # For linux, build from source
    elseif(UNIX AND NOT APPLE)
        find_package(PkgConfig QUIET)
        if (PKG_CONFIG_FOUND)
            pkg_check_modules(PC_SDL3 QUIET sdl3)
        endif()

        if (PC_SDL3_FOUND)
            message(STATUS "Found system-wide SDL3_image via pkg-config")
            find_package(SDL3_image CONFIG REQUIRED)
        else()
            message(STATUS "SDL3_image not found via pkg-config. Building from source...")
            include(FetchContent)
            FetchContent_Declare(
                sdl3_image_source
                GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
                GIT_TAG        release-${SDL3_IMAGE_VERSION}
            )
            FetchContent_MakeAvailable(sdl3_image_source)
        endif()
    else()
        message(FATAL_ERROR "Unsupported platform for automatic SDL3 setup.")
    endif()
endif()

if(TARGET SDL3_IMAGE::SDL3_IMAGE)
    message(STATUS "SDL3 setup complete. Use SDL3_IMAGE::SDL3_IMAGE target for linking.")
endif()
