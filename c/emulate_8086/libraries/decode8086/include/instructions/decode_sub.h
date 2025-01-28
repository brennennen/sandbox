

#ifndef DECODE_SUB_H
#define DECODE_SUB_H

#include <stdint.h>

#include "shared/include/instructions.h"

#include "libraries/decode8086/include/decode8086.h"

// MARK: SUB 1 - I_SUB
decode_result_t decode_sub(
    decoder_t* decoder,
    uint8_t byte1,
    sub_t* sub
);
void write_sub(
    sub_t* sub,
    char* buffer,
    int* index,
    int buffer_size
);

// MARK: SUB 2 - I_SUB_IMMEDIATE
// MARK: SUB 3 - I_SUB_IMMEDIATE_TO_AX


#endif // DECODE_SUB_H