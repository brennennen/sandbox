
set(SDL3_TTF_VERSION "3.2.2")

if(NOT TARGET SDL3_TTF::SDL3_TTF)
    message(STATUS "Attempting to find or build SDL3_ttf...")
    # For windows, download pre-compiled binaries and use those
    if(WIN32)
        set(SDL3_TTF_ARCHIVE_NAME "SDL3_ttf-devel-${SDL3_TTF_VERSION}-mingw.zip")
        # https://github.com/libsdl-org/SDL_ttf/releases/download/release-3.2.2/SDL3_ttf-devel-3.2.2-mingw.zip
        set(SDL3_TTF_URL "https://github.com/libsdl-org/SDL_ttf/releases/download/release-${SDL3_TTF_VERSION}/${SDL3_TTF_ARCHIVE_NAME}")
        FetchContent_Declare(
            sdl3_ttf_binaries
            URL          ${SDL3_TTF_URL}
            # cmake -E sha256sum SDL3_ttf-devel-3.2.2-mingw.zip
            URL_HASH     SHA256=e44e78d6dec080f1d96bd8ac1bac98ddc2574eff980d777c223c799c5a03b1ba
        )
        FetchContent_MakeAvailable(sdl3_ttf_binaries)
        set(SDL3_TTF_DIST_DIR "${sdl3_ttf_binaries_SOURCE_DIR}/x86_64-w64-mingw32")
        add_library(SDL3_ttf::SDL3_ttf INTERFACE IMPORTED)
        target_include_directories(SDL3_ttf::SDL3_ttf INTERFACE "${SDL3_TTF_DIST_DIR}/include")
        target_link_libraries(SDL3_ttf::SDL3_ttf INTERFACE "${SDL3_TTF_DIST_DIR}/lib/libSDL3_ttf.dll.a")
        set(SDL3_TTF_DLL_PATH "${SDL3_TTF_DIST_DIR}/bin/SDL3_ttf.dll")

    # For linux, build from source
    elseif(UNIX AND NOT APPLE)
        find_package(PkgConfig QUIET)
        if (PKG_CONFIG_FOUND)
            pkg_check_modules(PC_SDL3 QUIET sdl3)
        endif()

        if (PC_SDL3_FOUND)
            message(STATUS "Found system-wide SDL3_ttf via pkg-config")
            find_package(SDL3_ttf CONFIG REQUIRED)
        else()
            message(STATUS "SDL3_ttf not found via pkg-config. Building from source...")
            include(FetchContent)
            FetchContent_Declare(
                sdl3_ttf_source
                GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
                GIT_TAG        release-${SDL3_TTF_VERSION}
            )
            FetchContent_MakeAvailable(sdl3_ttf_source)
        endif()
    else()
        message(FATAL_ERROR "Unsupported platform for automatic SDL3 setup.")
    endif()
endif()

if(TARGET SDL3_TTF::SDL3_TTF)
    message(STATUS "SDL3 setup complete. Use SDL3_TTF::SDL3_TTF target for linking.")
endif()
