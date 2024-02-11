

include_directories(source include)
file(GLOB SOURCE_FILES source/*.c)
set(LIBRARIES
    lib_d
    lib_e
)

add_library(lib_c ${SOURCE_FILES})
target_link_libraries(lib_c LINK_PUBLIC ${LIBRARIES})


target_include_directories(lib_c PUBLIC include)