
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"

#include "libraries/emulate8086/include/emulate8086.h"

//
// MARK: SUB
//

// Use of this global "g_decoder" is to try and reduce the amount of code per test. It's reset
// after each test and has a large default instructions buffer.
static emulator_t g_emulator;

void emu_sub_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_t));
    emu_init(&g_emulator);
}

// MARK: 1. I_SUB Tests
Test(emu__I_SUB__tests, sub_1, .init = emu_sub_default_setup)
{
    uint8_t input[] = { 0x29, 0xd9 }; // sub cx, bx
    char output[32] = { 0x00 };
    g_emulator.registers.cx = 10;
    g_emulator.registers.bx = 7;
    cr_assert(SUCCESS == emu_emulate_chunk(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(3 == g_emulator.registers.cx);
    cr_assert(2 == g_emulator.registers.ip);
}

// MARK: I_SUB_IMMEDIATE Tests

// MARK: I_SUB_IMMEDIATE_TO_AX Tests
