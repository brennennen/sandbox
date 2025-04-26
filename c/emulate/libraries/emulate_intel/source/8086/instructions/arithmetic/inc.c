/**
 * `INC destination`
 * INC (Increment) adds one to the destination operand. The operand may be a byte or a word
 * and is treated as an unsigned binary number (see AAA and DAA). INC updates AF, OF, PF, SF,
 * and ZF; it does not affect CF (8086 Family Users Manual, page 2-35, pdf page ~50).
 *
 * This assembly instruction can be encoded into 2 different machine instructions.
 * The order of these encodings will follow the datasheet table 4-12 (8086 Family
 * Users Manual, page 4-23, pdf page ~165).
 *
 * INC 1 - I_INC
 * INC 2 - I_INC_REGISTER
 */

#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/result.h"

#include "8086/instruction_tags_8086.h"
#include "8086/emulate_8086.h"
#include "8086/emu_8086_registers.h"
#include "8086/decode_8086_utils.h"
#include "8086/decode_8086_shared.h"

#include "8086/instructions/arithmetic/inc.h"

// MARK: INC 1 - I_INC
emu_result_t decode_inc(
    emulator_8086_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
) {
    printf("TODO: decode_inc\n");
    return ER_FAILURE;
}

emu_result_t emu_inc(emulator_8086_t* emulator, uint8_t byte1) {
    printf("TODO: emu_inc\n");
    return ER_FAILURE;
}

// INC 2 - I_INC_REGISTER
emu_result_t decode_inc_register(
    emulator_8086_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
) {
    uint8_t reg = byte1 & 0b00000111;
    char* reg_string = map_register_field_encoding(reg);
    int written = snprintf(out_buffer + *index,  out_buffer_size - *index, "inc %s", reg_string);
    if (written < 0) {
        return ER_FAILURE;
    }
    *index += written;
    return ER_SUCCESS;
}

emu_result_t emu_inc_register(emulator_8086_t* emulator, uint8_t byte1) {
    printf("TODO: emu_inc_register\n");
    return ER_FAILURE;
}
