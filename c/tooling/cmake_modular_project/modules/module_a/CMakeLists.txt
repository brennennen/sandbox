

include_directories(source)
file(GLOB SOURCE_FILES source/*.c)
add_executable(module_a ${SOURCE_FILES})
