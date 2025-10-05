
set(CLAY_GIT_REPO "https://github.com/nicbarker/clay.git")
set(CLAY_GIT_COMMIT "588b93196cc7a182a024a9ef08ba8e352904a1bd")


if(NOT TARGET clay::clay)
    FetchContent_Declare(
        clay
        GIT_REPOSITORY      ${CLAY_GIT_REPO}
        GIT_TAG             ${CLAY_GIT_COMMIT}
    )
    FetchContent_Populate(clay)
    #FetchContent_MakeAvailable(clay EXCLUDE_FROM_ALL)

    # TODO: once clay's releases are more standardized, use below:
    # set(CLAY_VERSION "0.14")
    # set(CLAY_URL "https://github.com/nicbarker/clay/releases/download/v${CLAY_VERSION}/clay.h")
    # # cmake -E sha256sum clay.h
    # set(CLAY_SHA256 "c97241cc423af3fa11267978adce9cbb46274a2ad0709a5d4b2b1092dc27599d")
    # FetchContent_Declare(
    #     clay
    #     URL                 ${CLAY_URL}
    #     URL_HASH            SHA256=c97241cc423af3fa11267978adce9cbb46274a2ad0709a5d4b2b1092dc27599d
    #     DOWNLOAD_NO_EXTRACT TRUE # Important for single files
    # )
    # FetchContent_MakeAvailable(clay)

    add_library(clay::clay INTERFACE IMPORTED GLOBAL)
    target_include_directories(clay::clay INTERFACE 
        "${clay_SOURCE_DIR}"
    )
endif()

if(TARGET clay::clay)
    message(STATUS "clay setup complete. Use clay::clay target for linking.")
endif()
