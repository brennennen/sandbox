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
void emu_xchg_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_t));
    emu_init(&g_emulator);
}

// MARK: 1. I_EXCHANGE
Test(emu__I_EXCHANGE__tests, xchg1, .init = emu_xchg_default_setup)
{
    // Arrange
    uint8_t input[] = { 0x87, 0xcb }; // xchg bx, cx
    char output[32] = { 0x00 };
    g_emulator.registers.bx = 1;
    g_emulator.registers.cx = 2;

    // Act
    emu_result_t result = emu_emulate_chunk(&g_emulator, input, sizeof(input));

    // Assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(2 == g_emulator.registers.ip);
    cr_assert(2 == g_emulator.registers.bx);
    cr_assert(1 == g_emulator.registers.cx);
}

// MARK: 2. I_EXCHANGE_AX
Test(emu__I_EXCHANGE__tests, xchg_ax_1, .init = emu_xchg_default_setup)
{
    // Arrange
    uint8_t input[] = { 0x91 }; // xchg cx, ax
    char output[32] = { 0x00 };
    g_emulator.registers.ax = 1;
    g_emulator.registers.cx = 2;

    // Act
    emu_result_t result = emu_emulate_chunk(&g_emulator, input, sizeof(input));

    // Assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(1 == g_emulator.registers.ip);
    cr_assert(2 == g_emulator.registers.ax);
    cr_assert(1 == g_emulator.registers.cx);
}
