

#ifndef DECODE_CONDITIONAL_JUMPS_H
#define DECODE_CONDITIONAL_JUMPS_H

#include <stdint.h>

#include "shared/include/instructions.h"

#include "libraries/decode8086/include/decode8086.h"

// MARK: JE
decode_result_t decode_conditional_jump2(
    decoder_t* decoder,
    instruction_tag_t tag,
    uint8_t byte1,
    conditional_jump_t* conditional_jump
);

void write_conditional_jump(
    conditional_jump_t* conditional_jump,
    instruction_tag_t tag,
    char* buffer,
    int* index,
    int buffer_size
);

#endif // DECODE_CONDITIONAL_JUMPS_H
