
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "8086/instruction_tags_8086.h"

#include "8086/emulate_8086.h"

//
// MARK: SUB
//

// Use of this global "g_emulator" is to try and reduce the amount of code per test. It's reset
// after each test and has a large default instructions buffer.
static emulator_8086_t g_emulator;

void emu_sub_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_8086_t));
    emu_8086_init(&g_emulator);
}

// MARK: 1. I_SUB Tests
Test(emu__I_SUB__tests, sub_1, .init = emu_sub_default_setup)
{
    uint8_t input[] = { 0x29, 0xd9 }; // sub cx, bx
    g_emulator.registers.cx = 10;
    g_emulator.registers.bx = 7;
    cr_assert(SUCCESS == emu_8086_emulate_chunk(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(3 == g_emulator.registers.cx);
    cr_assert(PROGRAM_START + 3 == g_emulator.registers.ip);
}

// MARK: I_SUB_IMMEDIATE Tests

// MARK: I_SUB_IMMEDIATE_TO_AX Tests
