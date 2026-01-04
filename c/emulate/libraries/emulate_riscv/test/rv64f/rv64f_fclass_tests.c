
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/binary_utilities.h"
#include "rv64/rv64_emulate.h"
#include "rv64/modules/rv64f_float.h"

static rv64_emulator_t g_emulator;

void rv64_emu_fclass_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

// Test(emu_rv64_emulate__fclass__tests, fclass_1, .init = rv64_emu_fclass_default_setup)
// {
//     // arrange
//     g_emulator.harts[0].float32_registers[0] = 5.0f;
//     uint8_t input[] = { 0xe, 0x05 , 0x15, 0x53 }; // fclass.s a0, fa0
//     // act
//     result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
//     // assert
//     cr_assert(SUCCESS == result);
//     cr_assert(g_emulator.harts[0].registers[RV64_REG_A0] == RV64F_CLASS_POSITIVE_NORMAL);
//     cr_assert(1 == 2); // TODO: reminder to pick up here next time!
// }

// TODO: test the 9 other fclass scenarios at least once
