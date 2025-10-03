
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/binary_utilities.h"
#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_fmadd_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

Test(emu_rv64_emulate__fmadd__tests, fmadd_1, .init = rv64_emu_fmadd_default_setup)
{
    // arrange
    g_emulator.harts[0].float32_registers[1] = 5.0f;
    g_emulator.harts[0].float32_registers[2] = 2.0f;
    g_emulator.harts[0].float32_registers[3] = 3.0f;
    uint8_t input[] = { 0x43, 0xf0, 0x20, 0x18 }; // fmadd.s f0, f1, f2, f3 # f0 = (f1 * f2) + f3
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert_float_eq(13, g_emulator.harts[0].float32_registers[0], 1e-6); // (5 * 2) + 3 = 13
}

// TODO: write some tests that explore the rounding modes
