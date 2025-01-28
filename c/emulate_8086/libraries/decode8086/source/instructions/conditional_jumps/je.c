
#include "libraries/decode8086/include/instructions/conditional_jumps/je.h"






decode_result_t decode_jump_on_equal(
    decoder_t* decoder,
    uint8_t byte1,
    jump_on_equal_t* jump_on_equal
) {

    return DR_FAILURE;
}

void write_jump_on_equal(
    jump_on_equal_t* jump_on_equal,
    char* buffer,
    int* index,
    int buffer_size
) {

}