#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/binary_utilities.h"
#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_fsqrt_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

Test(emu_rv64_emulate__fsqrt__tests, fsqrt_1, .init = rv64_emu_fsqrt_default_setup)
{
    // arrange
    g_emulator.harts[0].float32_registers[1] = 36.0f;
    uint8_t input[] = { 0x53, 0xf0, 0x00, 0x58 }; // fsqrt.s f0, f1 # f0 = sqrt(f1)
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert_float_eq(6.0f, g_emulator.harts[0].float32_registers[0], 1e-6); // sqrt(36)
}
