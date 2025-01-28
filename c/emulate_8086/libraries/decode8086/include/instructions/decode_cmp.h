

#ifndef DECODE_CMP_H
#define DECODE_CMP_H

#include <stdint.h>

#include "shared/include/instructions.h"

#include "libraries/decode8086/include/decode8086.h"

// MARK: CMP 1 - I_CMP
decode_result_t decode_compare(
    decoder_t* decoder,
    uint8_t byte1,
    compare_t* compare
);
void write_compare(
    compare_t* sub,
    char* buffer,
    int* index,
    int buffer_size
);

// MARK: CMP 2 - I_COMPARE_IMMEDIATE
// MARK: CMP 3 - I_COMPARE_IMMEDIATE_TO_AX


#endif // DECODE_CMP_H