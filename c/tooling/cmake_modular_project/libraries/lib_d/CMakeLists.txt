

include_directories(source include)
file(GLOB SOURCE_FILES source/*.c)

add_library(lib_d ${SOURCE_FILES})
target_include_directories(lib_d PUBLIC include)