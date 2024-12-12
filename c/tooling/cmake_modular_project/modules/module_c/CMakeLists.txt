

include_directories(source)
file(GLOB SOURCE_FILES source/*.c)
set(LIBRARIES
    lib_c
)

add_executable(module_c ${SOURCE_FILES})
target_link_libraries(module_c LINK_PUBLIC ${LIBRARIES})