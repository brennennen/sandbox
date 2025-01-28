

#ifndef DECODE_ADD_H
#define DECODE_ADD_H

#include <stdint.h>

#include "shared/include/instructions.h"

#include "libraries/decode8086/include/decode8086.h"

// MARK: ADD 1 - I_ADD
decode_result_t decode_add(
    decoder_t* decoder,
    uint8_t byte1,
    add_t* add
);
void write_add(
    add_t* add,
    char* buffer,
    int* index,
    int buffer_size
);

// MARK: ADD 2 - I_ADD_IMMEDIATE
decode_result_t decode_add_immediate(
    decoder_t* decoder,
    uint8_t byte1,
    add_immediate_t* add
);
void write_add_immediate(
    add_immediate_t* add,
    char* buffer,
    int* index,
    int buffer_size
);

// MARK: ADD 3 - I_ADD_IMMEDIATE_TO_AX


#endif