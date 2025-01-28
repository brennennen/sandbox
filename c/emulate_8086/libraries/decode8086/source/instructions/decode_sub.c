




#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"

#include "libraries/decode8086/include/decode8086.h"
#include "libraries/decode8086/include/decode_utils.h"
#include "libraries/decode8086/include/decode_shared.h"

#include "libraries/decode8086/include/instructions/decode_sub.h"



decode_result_t decode_sub(
    decoder_t* decoder,
    uint8_t byte1,
    sub_t* sub
) {
    return decode__opcode_d_w__mod_reg_rm__disp_lo__disp_hi(
        decoder, byte1, &sub->fields1, &sub->fields2, &sub->displacement
    );
}

void write_sub(
    sub_t* sub,
    char* buffer,
    int* index,
    int buffer_size
) {
    direction_t direction = (sub->fields1 & 0b00000010) >> 1;
    wide_t wide = sub->fields1 & 0b00000001;
    mod_t mod = (sub->fields2 & 0b11000000) >> 6;
    reg_t reg = (sub->fields2 & 0b00111000) >> 3;
    uint8_t rm = sub->fields2 & 0b00000111;

    write__common_register_or_memory_with_register_or_memory(direction, wide, mod, reg, rm,
        sub->displacement, "sub", 3, buffer, index, buffer_size);
}
