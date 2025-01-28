

#ifndef DECODE_CMP_H
#define DECODE_CMP_H

#include <stdint.h>

#include "shared/include/instructions.h"

#include "libraries/decode8086/include/decode8086.h"

// MARK: JE
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

#endif // DECODE_CMP_H
