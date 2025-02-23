/**
 * Emulation tests for the "add" instruction.
 */
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"

#include "libraries/emulate8086/include/emulate8086.h"
#include "libraries/emulate8086/include/emu_registers.h"


// Use of this global "g_emulator" is to try and reduce the amount of code per test. It's reset
// after each test and has a large default instructions buffer.
static emulator_t g_emulator;
void emu_add_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_t));
    emu_init(&g_emulator);
}

// MARK: 1. I_ADD
// TODO

// MARK: 2. I_ADD_IMMEDIATE
Test(emu__I_ADD_IMMEDIATE__tests, add1, .init = emu_add_default_setup)
{
    char* expected = "add cx, 5\n";
    uint8_t input[] = { 0x83, 0xc1, 0x05 }; // 0b10000011
    char output[32] = { 0x00 };
    cr_assert(SUCCESS == emu_emulate_chunk(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(5 == g_emulator.registers.cx,
        "expected:\n'%d'\n\nactual:\n'%d'\n",
        5, g_emulator.registers.cx);
    uint16_t zf_flag = emu_reg_get_flag(g_emulator.registers.flags, FLAG_ZF_MASK);
    cr_assert(0 == zf_flag,
        "expected:\n'%d'\n\nactual:\n'%d'\n", 0, zf_flag);
    // TODO: check carry, zero, parity, etc. flags?
}

Test(emu__I_ADD_IMMEDIATE__tests, add2, .init = emu_add_default_setup)
{
    char* expected = "add cx, 0\n";
    uint8_t input[] = { 0x83, 0xc1, 0x00 }; // 0b10000011
    char output[32] = { 0x00 };
    cr_assert(SUCCESS == emu_emulate_chunk(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(0 == g_emulator.registers.cx,
        "expected:\n'%d'\n\nactual:\n'%d'\n",
        5, g_emulator.registers.cx);
    uint16_t zf_flag = emu_reg_get_flag(g_emulator.registers.flags, FLAG_ZF_MASK);
    cr_assert(FLAG_ZF_MASK == zf_flag,
        "expected:\n'%d'\n\nactual:\n'%d'\n", FLAG_ZF_MASK, zf_flag);
    // TODO: check carry, zero, parity, etc. flags?
}

// MARK: 3. I_ADD_IMMEDIATE_TO_AX
// TODO
