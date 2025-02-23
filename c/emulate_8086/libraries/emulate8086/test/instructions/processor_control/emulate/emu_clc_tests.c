/**
 * Emulation tests for the "clc" instruction.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"

#include "libraries/emulate8086/include/emulate8086.h"
#include "libraries/emulate8086/include/emu_registers.h"


// Use of this global "g_emulator" is to try and reduce the amount of code per test. It's reset
// after each test and has a large default instructions buffer.
static emulator_t g_emulator;
void emu_clc_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_t));
    emu_init(&g_emulator);
}

// MARK: CLC
Test(emu__CLC__tests, clc1, .init = emu_clc_default_setup)
{
    char* expected = "clc\n";
    uint8_t input[] = { 0xF8 }; // 0b11111000
    char output[32] = { 0x00 };
    emu_reg_set_flag(&g_emulator.registers.flags, FLAG_CF_MASK);
    cr_assert(SUCCESS == emu_emulate_chunk(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.instructions_count);
    uint16_t cf = emu_reg_get_flag(g_emulator.registers.flags, FLAG_CF_MASK);
    cr_assert(0 == cf);
}
