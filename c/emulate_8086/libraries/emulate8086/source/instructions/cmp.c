




#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"

#include "libraries/emulate8086/include/emulate8086.h"
#include "libraries/emulate8086/include/decode_utils.h"
#include "libraries/emulate8086/include/decode_shared.h"

#include "libraries/emulate8086/include/instructions/cmp.h"



emu_result_t decode_compare(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
) {
    direction_t direction = 0;
    wide_t wide = 0;
    mod_t mod = 0;
    uint8_t reg = 0;
    uint8_t rm = 0;
    uint16_t displacement = 0;

    emu_result_t result = decode__opcode_d_w__mod_reg_rm__disp_lo__disp_hi(
        emulator, byte1, &direction, &wide, &mod, &reg, &rm, &displacement
    );

    write__common_register_or_memory_with_register_or_memory(
        direction, wide, mod, reg, rm, displacement,
        "cmp", 3, out_buffer, index, out_buffer_size
    );
    return result;
}

emu_result_t emu_compare(emulator_t* emulator, uint8_t byte1) {
    direction_t direction = 0;
    wide_t wide = 0;
    mod_t mod = 0;
    uint8_t reg = 0;
    uint8_t rm = 0;
    uint16_t displacement = 0;

    emu_result_t result = decode__opcode_d_w__mod_reg_rm__disp_lo__disp_hi(
        emulator, byte1, &direction, &wide, &mod, &reg, &rm, &displacement
    );

    // TODO

    return ER_FAILURE;
}
