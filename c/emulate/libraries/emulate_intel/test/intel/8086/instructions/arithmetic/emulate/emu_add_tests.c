/**
 * Emulation tests for the "add" instruction.
 */
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "8086/instruction_tags_8086.h"

#include "8086/emu_8086_registers.h"
#include "8086/emulate_8086.h"

// Use of this global "g_emulator" is to try and reduce the amount of code per test. It's reset
// after each test and has a large default instructions buffer.
static emulator_8086_t g_emulator;

void emu_add_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_8086_t));
    emu_8086_init(&g_emulator);
}

// MARK: 1. I_ADD
Test(emu__I_ADD__tests, add1, .init = emu_add_default_setup) {
    // Arrange
    uint8_t input[] = {0x01, 0xd9};  // add cx, bx
    g_emulator.registers.cx = 3;
    g_emulator.registers.bx = 4;

    // Act
    emu_result_t result = emu_8086_emulate_chunk(&g_emulator, input, sizeof(input));

    // Assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(PROGRAM_START + 3 == g_emulator.registers.ip);
    cr_assert(7 == g_emulator.registers.cx);
    cr_assert(0 == emu_reg_get_flag(g_emulator.registers.flags, FLAG_ZF_MASK));
}

// MARK: 2. I_ADD_IMMEDIATE
Test(emu__I_ADD_IMMEDIATE__tests, add_immediate_1, .init = emu_add_default_setup) {
    uint8_t input[] = {0x83, 0xc1, 0x05};  // add cx, 5
    cr_assert(SUCCESS == emu_8086_emulate_chunk(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(PROGRAM_START + 4 == g_emulator.registers.ip);
    cr_assert(5 == g_emulator.registers.cx);
    cr_assert(0 == emu_reg_get_flag(g_emulator.registers.flags, FLAG_ZF_MASK));
    // TODO: check carry, zero, parity, etc. flags?
}

Test(emu__I_ADD_IMMEDIATE__tests, add_immediate_2, .init = emu_add_default_setup) {
    uint8_t input[] = {0x83, 0xc1, 0x00};  // add cx, 0
    cr_assert(SUCCESS == emu_8086_emulate_chunk(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(PROGRAM_START + 4 == g_emulator.registers.ip);
    cr_assert(0 == g_emulator.registers.cx);
    cr_assert(FLAG_ZF_MASK == emu_reg_get_flag(g_emulator.registers.flags, FLAG_ZF_MASK));
    // TODO: check carry, zero, parity, etc. flags?
}

Test(emu__I_ADD_IMMEDIATE__tests, add_immediate_3, .init = emu_add_default_setup) {
    uint8_t input[] = {0x83, 0xc6, 0x02};  // add si, 2
    cr_assert(SUCCESS == emu_8086_emulate_chunk(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(PROGRAM_START + 4 == g_emulator.registers.ip);
    cr_assert(2 == g_emulator.registers.si);
    // TODO: check carry, zero, parity, etc. flags?
}

// MARK: 3. I_ADD_IMMEDIATE_TO_AX
// TODO
