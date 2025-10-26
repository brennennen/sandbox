
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/binary_utilities.h"
#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_fadd_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

Test(emu_rv64_emulate__fadd__tests, fadd_1, .init = rv64_emu_fadd_default_setup)
{
    // arrange
    g_emulator.harts[0].float32_registers[1] = 5.0f;
    g_emulator.harts[0].float32_registers[2] = 2.0f;
    uint8_t input[] = { 0x53, 0xf0, 0x20, 0x00 }; // fadd.s f0, f1, f2  # f0 = f1 + f2
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert_float_eq(7.0f, g_emulator.harts[0].float32_registers[0], 1e-6); // 5 + 2
}

// TODO: write some tests that explore the rounding modes


// MARK: fflag tests

Test(emu_rv64_emulate__fadd__tests, fadd_overflow, .init = rv64_emu_fadd_default_setup)
{
    // arrange
    g_emulator.harts[0].float32_registers[1] = FLT_MAX;
    g_emulator.harts[0].float32_registers[2] = FLT_MAX;
    uint8_t input[] = { 0x53, 0xf0, 0x20, 0x00 }; // fadd.s f0, f1, f2  # f0 = f1 + f2
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(isinf(g_emulator.harts[0].float32_registers[0]));
    cr_assert(g_emulator.harts[0].float32_registers[0] > 0); // positive infinity
    int fflags = (0b11111 & g_emulator.harts[0].csrs.fcsr);
    int expected_fflags = (1 << RV64F_FCSR_INEXACT) | (1 << RV64F_FCSR_OVERFLOW);
    cr_assert(expected_fflags == fflags);
}

Test(emu_rv64_emulate__fadd__tests, fadd_qnan, .init = rv64_emu_fadd_default_setup)
{
    // arrange
    g_emulator.harts[0].float32_registers[1] = INFINITY;
    g_emulator.harts[0].float32_registers[2] = -INFINITY;
    uint8_t input[] = { 0x53, 0xf0, 0x20, 0x00 }; // fadd.s f0, f1, f2  # f0 = f1 + f2
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(isnan(g_emulator.harts[0].float32_registers[0]));
    int fflags = (0b11111 & g_emulator.harts[0].csrs.fcsr);
    cr_assert((1 << RV64F_FCSR_INVALID_OPERATION) == fflags);
}

Test(emu_rv64_emulate__fadd__tests, fadd_inexact, .init = rv64_emu_fadd_default_setup)
{
    // arrange
    g_emulator.harts[0].float32_registers[1] = 0.1f;
    g_emulator.harts[0].float32_registers[2] = 0.2f;
    uint8_t input[] = { 0x53, 0xf0, 0x20, 0x00 }; // fadd.s f0, f1, f2  # f0 = f1 + f2
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert_float_eq(0.3f, g_emulator.harts[0].float32_registers[0], 1e-9);
    int fflags = (0b11111 & g_emulator.harts[0].csrs.fcsr);
    cr_assert((1 << RV64F_FCSR_INEXACT) == fflags);
}
