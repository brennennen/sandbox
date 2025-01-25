

#ifndef DECODE_ADD_H
#define DECODE_ADD_H

#include <stdint.h>

#include "shared/include/instructions.h"

#include "libraries/decode8086/include/decode8086.h"

// MARK: ADD 1 - I_ADD_REGISTER_OR_MEMORY_WITH_REGISTER_TO_EITHER
decode_result_t decode__add_register_or_memory_with_register_to_either(
    decoder_t* decoder,
    uint8_t byte1,
    add_register_or_memory_with_register_to_either_t* add
);
void write__add_register_or_memory_with_register_to_either(
    add_register_or_memory_with_register_to_either_t* add,
    char* buffer,
    int* index,
    int buffer_size
);

// MARK: ADD 2 - I_ADD_IMMEDIATE_TO_REGISTER_OR_MEMORY
// MARK: ADD 3 - I_ADD_IMMEDIATE_TO_ACCUMULATOR


#endif