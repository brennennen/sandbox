/**
 * Emulation tests for the "and" instruction.
 */
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"

#include "libraries/emulate8086/include/emulate8086.h"
#include "libraries/emulate8086/include/emu_registers.h"
#include "libraries/emulate8086/include/logger.h"

static emulator_t g_emulator;
void emu_and_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_t));
    emu_init(&g_emulator);
}

// MARK: I_AND



// MARK: I_AND_IMMEDIATE
// I_AND_IMMEDIATE - register
Test(emu__I_AND_IMMEDIATE__tests, and__register__m8, .init = emu_and_default_setup)
{
    uint8_t input[] = { 0x80, 0xe1, 0x0c }; // "and cl, 12" - 0b10000000 0b11100001 ...
    g_emulator.registers.cx = 5; // 0b0101
    emu_result_t result = emu_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(4 == g_emulator.registers.cx); // 0b0101 & 0b1100 = 0b0100
}

Test(emu__I_AND_IMMEDIATE__tests, and__register__m16, .init = emu_and_default_setup)
{
    uint8_t input[] = { 0x83, 0xe1, 0x0c }; // "and cx, 12" - 0b10000011 0b11100001 ...
    g_emulator.registers.cx = 5; // 0b0101
    emu_result_t result = emu_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(4 == g_emulator.registers.cx); // 0b0101 & 0b1100 = 0b0100
}

Test(emu__I_AND_IMMEDIATE__tests, and__register__m16_2, .init = emu_and_default_setup)
{
    uint8_t input[] = { 0x81, 0xe1, 0xf1, 0xf0 }; // "and cx, 0xF0F1"
    g_emulator.registers.cx = 0x1111;
    emu_result_t result = emu_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(0x1011 == g_emulator.registers.cx); // 0xF0F1 & 0x1111 = 0x1011
}

Test(emu__I_AND_IMMEDIATE__tests, and__register__m16_3, .init = emu_and_default_setup)
{
    // NOTE: 3 bytes instead of 4, 0xFFF0 is just 0xF0 because of "signed bit extension".
    uint8_t input[] = { 0x83, 0xe1, 0xf0 }; // "and cx, 0xFFF0"
    g_emulator.registers.cx = 0x1111;
    emu_result_t result = emu_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    printf("cx: %x\n", g_emulator.registers.cx);
    cr_assert(0x1110 == g_emulator.registers.cx); // 0xFFF0 & 0x1111 = 0x1110
}

Test(emu__I_AND_IMMEDIATE__tests, and__register__m16_signed_extension, .init = emu_and_default_setup)
{
    // NOTE: 3 bytes instead of 4, 0xFFF0 is just 0xF0 because of "signed bit extension".
    uint8_t input[] = { 0x83, 0xe1, 0x80 }; // "and cx, 0xFF80"
    g_emulator.registers.cx = 0x8888;
    emu_result_t result = emu_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    printf("cx: %x\n", g_emulator.registers.cx);
    cr_assert(0x8880 == g_emulator.registers.cx); // 0xFF80 & 0x8888 = 0x8880
}

// I_AND_IMMEDIATE - direct access
// Test(emu__I_AND_IMMEDIATE__tests, and__direct_access__m8, .init = emu_and_default_setup)
// {
//     uint8_t input[] = { 0x80, 0x26, 0xe8, 0x03, 0x0c }; // "and byte [1000], 12" - 0b10000000 ...
//     g_emulator.memory[1000] = 5; // 0b0101
//     emu_result_t result = emu_emulate_chunk(&g_emulator, input, sizeof(input));
//     cr_assert(SUCCESS == result);
//     cr_assert(1 == g_emulator.instructions_count);
//     cr_assert(4 == g_emulator.memory[1000]); // 0b0101 & 0b1100 = 0b0100
// }

// I_AND_IMMEDIATE - memory
// I_AND_IMMEDIATE - memory + displacement


// MARK: I_AND_IMMEDIATE_TO_AX


