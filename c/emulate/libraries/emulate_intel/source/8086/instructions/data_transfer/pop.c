/**
 * `POP destination`
 * POP transfers the word at the current top of stack (pointed to by SP) to the
 * destination operand, and then increments SP by two to point to the new top of stack.
 * POP can be used to move temporary variables from the stack to registers or memory
 * (8086 Family Users Manual, page 2-31, pdf page ~46).
 *
 * This instruction can be mapped into 3 different opcode encodings. The order of these
 * encodings will follow the datasheet table 4-12 (8086 Family Users Manual, page 4-22,
 * pdf page ~164).
 *
 * 1. I_POP
 * 2. I_POP_REGISTER
 * 3. I_POP_SEGMENT_REGISTER
 *
 */

#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/result.h"

#include "8086/emulate_8086.h"
#include "8086/emu_8086_registers.h"
#include "8086/decode_8086_utils.h"
#include "8086/decode_8086_shared.h"

#include "8086/instructions/data_transfer/pop.h"

// MARK: 1. I_POP

// MARK: 2. I_POP_REGISTER
emu_result_t decode_pop_register(
    emulator_8086_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
) {
    uint8_t reg = byte1 & 0b00000111;
    char* reg_string = map_register_field_encoding(reg);;
    int written = snprintf(out_buffer + *index,  out_buffer_size - *index, "pop %s", reg_string);
    if (written < 0) {
        return ER_FAILURE;
    }
    *index += written;
    return ER_SUCCESS;
}

emu_result_t emu_pop_register(emulator_8086_t* emulator, uint8_t byte1) {
    uint8_t reg = byte1 & 0b00000111;

    //emulator->registers.ip += 1;

    return ER_FAILURE;
}

 // MARK: 3. I_POP_SEGMENT_REGISTER

