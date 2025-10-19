
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/binary_utilities.h"
#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_fcvt_w_s_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

// MARK: round nearest ties even
Test(emu_rv64_emulate__fcvt_w_s__tests, fcvt_w_s_positive_rne, .init = rv64_emu_fcvt_w_s_default_setup)
{
    // arrange
    g_emulator.harts[0].float32_registers[0] = 123.5;
    uint8_t input[] = { 0x53, 0x05, 0x00, 0xc0 }; // fcvt.w.s a0, ft0, rne
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(124 == g_emulator.harts[0].registers[RV64_REG_A0]);
}

Test(emu_rv64_emulate__fcvt_w_s__tests, fcvt_w_s_positive_rne_2, .init = rv64_emu_fcvt_w_s_default_setup)
{
    // arrange
    g_emulator.harts[0].float32_registers[0] = 124.5;
    uint8_t input[] = { 0x53, 0x05, 0x00, 0xc0 }; // fcvt.w.s a0, ft0, rne
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(124 == g_emulator.harts[0].registers[RV64_REG_A0]);
}

Test(emu_rv64_emulate__fcvt_w_s__tests, fcvt_w_negative_rne, .init = rv64_emu_fcvt_w_s_default_setup)
{
    // arrange
    g_emulator.harts[0].float32_registers[0] = -123.5;
    uint8_t input[] = { 0x53, 0x05, 0x00, 0xc0 }; // fcvt.w.s a0, ft0, rne
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(-124 == g_emulator.harts[0].registers[RV64_REG_A0]);
}

Test(emu_rv64_emulate__fcvt_w_s__tests, fcvt_w_negative_rne_2, .init = rv64_emu_fcvt_w_s_default_setup)
{
    // arrange
    g_emulator.harts[0].float32_registers[0] = -124.5;
    uint8_t input[] = { 0x53, 0x05, 0x00, 0xc0 }; // fcvt.w.s a0, ft0, rne
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(-124 == g_emulator.harts[0].registers[RV64_REG_A0]);
}

// MARK: round toward zero
Test(emu_rv64_emulate__fcvt_w_s__tests, fcvt_w_s_positive_rtz, .init = rv64_emu_fcvt_w_s_default_setup)
{
    // arrange
    g_emulator.harts[0].float32_registers[0] = 123.45;
    uint8_t input[] = { 0x53, 0x15, 0x00, 0xc0 }; // fcvt.w.s a0, ft0, rtz
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(123 == g_emulator.harts[0].registers[RV64_REG_A0]);
}

Test(emu_rv64_emulate__fcvt_w_s__tests, fcvt_w_negative_rtz, .init = rv64_emu_fcvt_w_s_default_setup)
{
    // arrange
    g_emulator.harts[0].float32_registers[0] = -123.45;
    uint8_t input[] = { 0x53, 0x15, 0x00, 0xc0 }; // fcvt.w.s a0, ft0, rtz
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(-123 == g_emulator.harts[0].registers[RV64_REG_A0]);
}

// MARK: round down
Test(emu_rv64_emulate__fcvt_w_s__tests, fcvt_w_s_positive_rdn, .init = rv64_emu_fcvt_w_s_default_setup)
{
    // arrange
    g_emulator.harts[0].float32_registers[0] = 123.45;
    uint8_t input[] = { 0x53, 0x25, 0x00, 0xc0 }; // fcvt.w.s a0, ft0, rdn
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(123 == g_emulator.harts[0].registers[RV64_REG_A0]);
}

Test(emu_rv64_emulate__fcvt_w_s__tests, fcvt_w_negative_rdn, .init = rv64_emu_fcvt_w_s_default_setup)
{
    // arrange
    g_emulator.harts[0].float32_registers[0] = -123.45;
    uint8_t input[] = { 0x53, 0x25, 0x00, 0xc0 }; // fcvt.w.s a0, ft0, rdn
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(-124 == g_emulator.harts[0].registers[RV64_REG_A0]);
}

// MARK: round up
Test(emu_rv64_emulate__fcvt_w_s__tests, fcvt_w_s_positive_rup, .init = rv64_emu_fcvt_w_s_default_setup)
{
    // arrange
    g_emulator.harts[0].float32_registers[0] = 123.45;
    uint8_t input[] = { 0x53, 0x35, 0x00, 0xc0 }; // fcvt.w.s a0, ft0, rup
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(124 == g_emulator.harts[0].registers[RV64_REG_A0]);
}

Test(emu_rv64_emulate__fcvt_w_s__tests, fcvt_w_negative_rup, .init = rv64_emu_fcvt_w_s_default_setup)
{
    // arrange
    g_emulator.harts[0].float32_registers[0] = -123.45;
    uint8_t input[] = { 0x53, 0x35, 0x00, 0xc0 }; // fcvt.w.s a0, ft0, rup
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(-123 == g_emulator.harts[0].registers[RV64_REG_A0]);
}

// MARK: round nearest ties max magnitude
Test(emu_rv64_emulate__fcvt_w_s__tests, fcvt_w_s_positive_rmm, .init = rv64_emu_fcvt_w_s_default_setup)
{
    // arrange
    g_emulator.harts[0].float32_registers[0] = 123.5;
    uint8_t input[] = { 0x53, 0x45, 0x00, 0xc0 }; // fcvt.w.s a0, ft0, rmm
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(124 == g_emulator.harts[0].registers[RV64_REG_A0]);
}

Test(emu_rv64_emulate__fcvt_w_s__tests, fcvt_w_s_positive_rmm_2, .init = rv64_emu_fcvt_w_s_default_setup)
{
    // arrange
    g_emulator.harts[0].float32_registers[0] = 124.5;
    uint8_t input[] = { 0x53, 0x45, 0x00, 0xc0 }; // fcvt.w.s a0, ft0, rmm
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(125 == g_emulator.harts[0].registers[RV64_REG_A0]);
}

Test(emu_rv64_emulate__fcvt_w_s__tests, fcvt_w_negative_rmm, .init = rv64_emu_fcvt_w_s_default_setup)
{
    // arrange
    g_emulator.harts[0].float32_registers[0] = -123.5;
    uint8_t input[] = { 0x53, 0x45, 0x00, 0xc0 }; // fcvt.w.s a0, ft0, rmm
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(-124 == g_emulator.harts[0].registers[RV64_REG_A0]);
}

Test(emu_rv64_emulate__fcvt_w_s__tests, fcvt_w_negative_rmm_2, .init = rv64_emu_fcvt_w_s_default_setup)
{
    // arrange
    g_emulator.harts[0].float32_registers[0] = -124.5;
    uint8_t input[] = { 0x53, 0x45, 0x00, 0xc0 }; // fcvt.w.s a0, ft0, rmm
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(-125 == g_emulator.harts[0].registers[RV64_REG_A0]);
}
