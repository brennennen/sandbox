
set(CLAY_GIT_REPO "https://github.com/nicbarker/clay.git")
set(CLAY_GIT_COMMIT "588b93196cc7a182a024a9ef08ba8e352904a1bd")

if(NOT TARGET clay::clay)
    FetchContent_Declare(
        clay
        GIT_REPOSITORY      ${CLAY_GIT_REPO}
        GIT_TAG             ${CLAY_GIT_COMMIT}
    )
    FetchContent_Populate(clay)
    add_library(clay::clay INTERFACE IMPORTED GLOBAL)
    target_include_directories(clay::clay INTERFACE 
        "${clay_SOURCE_DIR}"
    )
endif()

if(TARGET clay::clay)
    message(STATUS "clay setup complete. Use clay::clay target for linking.")
endif()
