
#ifndef I_JE_H
#define I_JE_H

#include <stdint.h>

#include "shared/include/instructions.h"

#include "libraries/decode8086/include/decode8086.h"

// MARK: JE/JZ 1 - I_JUMP_ON_EQUAL
decode_result_t decode_jump_on_equal(
    decoder_t* decoder,
    uint8_t byte1,
    jump_on_equal_t* jump_on_equal
);
void write_jump_on_equal(
    jump_on_equal_t* jump_on_equal,
    char* buffer,
    int* index,
    int buffer_size
);

#endif // I_JE_H
