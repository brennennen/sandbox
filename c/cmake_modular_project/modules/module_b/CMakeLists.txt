

include_directories(source)
file(GLOB SOURCE_FILES source/*.c)
set(LIBRARIES
    lib_a
    lib_b
)

add_executable(module_b ${SOURCE_FILES})
target_link_libraries(module_b LINK_PUBLIC ${LIBRARIES})

if (testing_enabled)
    include_directories(test)
endif()