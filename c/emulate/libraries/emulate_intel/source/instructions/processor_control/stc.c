/**
 * `STC`
 * STC (Set Carry flag) sets CF to 1 and affects no other flags (8086 Family
 * Users Manual, page 2-47, pdf page ~62).
 *
 * STC has 1 machine instruction on 8086. See datasheet table 4-12 (8086 Family
 * Users Manual, page 4-27, pdf page ~169).
 *
 */

#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"

#include "libraries/emulate_intel/include/emulate.h"
#include "libraries/emulate_intel/include/emu_registers.h"
#include "libraries/emulate_intel/include/decode_utils.h"
#include "libraries/emulate_intel/include/decode_shared.h"

#include "libraries/emulate_intel/include/instructions/processor_control/clc.h"


// MARK: STC
emu_result_t decode_stc(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size)
{
    if (*index + 4 <= out_buffer_size) {
        out_buffer[*index] = 's';
        out_buffer[*index + 1] = 't';
        out_buffer[*index + 2] = 'c';
        out_buffer[*index + 3] = '\n';
        *index += 4;
        return ER_SUCCESS;
    }
    return ER_FAILURE;
}

emu_result_t emu_stc(emulator_t* emulator, uint8_t byte1) {
    emu_reg_set_flag(&emulator->registers.flags, FLAG_CF_MASK);
    emulator->registers.ip += 1;
    return ER_SUCCESS;
}
