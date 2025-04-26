/**
 * `CLC`
 * CLC (Clear Carry flag) zeroes the carry flag (CF) and affects no other flags.
 * It (and CMC and STC) is useful in conjunction with the TCL and RCR instructions
 * (8086 Family Users Manual, page 2-47, pdf page ~62).
 *
 * CLC has 1 machine instruction on 8086. See datasheet table 4-12 (8086 Family
 * Users Manual, page 4-27, pdf page ~169).
 *
 */

#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/result.h"

#include "8086/emulate_8086.h"
#include "8086/emu_8086_registers.h"
#include "8086/decode_8086_utils.h"
#include "8086/decode_8086_shared.h"

#include "8086/instructions/processor_control/cmc.h"


// MARK: CMC
emu_result_t decode_cmc(
    emulator_8086_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size)
{
    if (*index + 4 <= out_buffer_size) {
        out_buffer[*index] = 'c';
        out_buffer[*index + 1] = 'm';
        out_buffer[*index + 2] = 'c';
        out_buffer[*index + 3] = '\n';
        *index += 4;
        return ER_SUCCESS;
    }
    return ER_FAILURE;
}

emu_result_t emu_cmc(emulator_8086_t* emulator, uint8_t byte1) {
    emu_reg_toggle_flag(&emulator->registers.flags, FLAG_CF_MASK);
    emulator->registers.ip += 1;
    return ER_SUCCESS;
}
