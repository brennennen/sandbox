/**
 * Emulation tests for the "not" instruction.
 */
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"

#include "libraries/emulate8086/include/emulate8086.h"
#include "libraries/emulate8086/include/emu_registers.h"
#include "libraries/emulate8086/include/logger.h"

static emulator_t g_emulator;
void emu_not_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_t));
    emu_init(&g_emulator);
}

// MARK: 1. I_NOT
Test(emu__I_NOT__tests, not__register__m8, .init = emu_not_default_setup)
{
    uint8_t input[] = { 0xf6, 0xd0 }; // "not al" - 0b11110110 0b11010000
    g_emulator.registers.ax = 5; // 0b0101
    emu_result_t result = emu_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(0xFA == g_emulator.registers.ax);
}

Test(emu__I_NOT__tests, not__register__m16, .init = emu_not_default_setup)
{
    uint8_t input[] = { 0xf7, 0xd0 }; // "not ax" - 0b11110111 0b11010000
    g_emulator.registers.ax = 5; // 0b0101
    emu_result_t result = emu_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(0xFFFA == g_emulator.registers.ax);
}

Test(emu__I_NOT__tests, not__direct_access__m8, .init = emu_not_default_setup)
{
    uint8_t input[] = { 0xf6, 0x16, 0xe8, 0x03 }; // "not byte [1000]"
    g_emulator.memory[1000] = 10; // 0b1010
    emu_result_t result = emu_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(0xF5 == g_emulator.memory[1000]);
}

Test(emu__I_NOT__tests, not__direct_access__m16, .init = emu_not_default_setup)
{
    uint8_t input[] = { 0xf7, 0x16, 0xe8, 0x03 }; // "not word [1000]"
    emu_memory_set_uint16(&g_emulator, 1000, 513);
    emu_result_t result = emu_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    uint16_t memory_val = 0;
    emu_memory_get_uint16(&g_emulator, 1000, &memory_val);
    cr_assert(0xFDFE == memory_val);
}

Test(emu__I_NOT__tests, not__memory__m8, .init = emu_not_default_setup)
{
    uint8_t input[] = { 0xf6, 0x17 }; // "not byte [bx]" - 0b11110110 0b00010111
    g_emulator.registers.bx = 1000;
    g_emulator.memory[1000] = 10; // 0b1010
    emu_result_t result = emu_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(0xF5 == g_emulator.memory[1000]); // 0b11110101
}

Test(emu__I_NOT__tests, not__memory__m16, .init = emu_not_default_setup)
{
    uint8_t input[] = { 0xf7, 0x17 }; // "not word [bx]"
    g_emulator.registers.bx = 1000;
    emu_memory_set_uint16(&g_emulator, 1000, 513);
    emu_result_t result = emu_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    uint16_t memory_val = 0;
    emu_memory_get_uint16(&g_emulator, 1000, &memory_val);
    cr_assert(0xFDFE == memory_val);
}

Test(emu__I_NOT__tests, not__memory_and_displacement__m8, .init = emu_not_default_setup)
{
    uint8_t input[] = { 0xf6, 0x97, 0xe8, 0x03 }; // "not byte [bx + 1000]"
    g_emulator.registers.bx = 500;
    g_emulator.memory[1500] = 10;
    LOGMEM(LOG_INFO, g_emulator.memory, 1500, sizeof(g_emulator.memory), "[1500] before");
    emu_result_t result = emu_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(PROGRAM_START + 5 == g_emulator.registers.ip);
    uint16_t memory_val = 0;
    LOGMEM(LOG_INFO, g_emulator.memory, 1500, sizeof(g_emulator.memory), "[1500] after");
    cr_assert(0xF5 == g_emulator.memory[1500]);
}

Test(emu__I_NOT__tests, not__memory_and_displacement__m16, .init = emu_not_default_setup)
{
    uint8_t input[] = { 0xf7, 0x97, 0xe8, 0x03 }; // "not word [bx + 1000]"
    g_emulator.registers.bx = 500;
    emu_memory_set_uint16(&g_emulator, 1500, 513);
    emu_result_t result = emu_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(PROGRAM_START + 5 == g_emulator.registers.ip); // 5
    uint16_t memory_val = 0;
    emu_memory_get_uint16(&g_emulator, 1500, &memory_val);
    cr_assert(0xFDFE == memory_val);
}
