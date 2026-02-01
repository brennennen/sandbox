/**
 * Emulation tests for the "and" instruction.
 */
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "8086/instruction_tags_8086.h"

#include "8086/emu_8086_registers.h"
#include "8086/emulate_8086.h"
#include "logger.h"

static emulator_8086_t g_emulator;

void emu_and_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_8086_t));
    emu_8086_init(&g_emulator);
}

// MARK: I_AND
Test(emu__I_AND__tests, and__register__m8, .init = emu_and_default_setup) {
    uint8_t input[] = {0x20, 0xcb};  // "and bl, cl"
    g_emulator.registers.bx = 5;     // 0b0101
    g_emulator.registers.cx = 12;    // 0b1100
    emu_result_t result = emu_8086_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(4 == g_emulator.registers.bx);  // 0b0101 & 0b1100 = 0b0100
}

// Test(emu__I_AND__tests, and__register__m32, .init = emu_and_default_setup)
// {
//     uint8_t input[] = { 0x21, 0xcb }; // bits 32 "and ebx, ecx" - 0b00100001 0b11001011
// }

// Test(emu__I_AND__tests, and__register__m64, .init = emu_and_default_setup)
// {
//     uint8_t input[] = { 0x48, 0x21, 0xcb }; // bits 64 "and rbx, rcx"
// }

Test(emu__I_AND__tests, and__direct_access__m8, .init = emu_and_default_setup) {
    uint8_t input[] = {0x22, 0x1e, 0xe8, 0x03};  // "and bl, [1000]"
    g_emulator.registers.bx = 5;                 // 0b0101
    g_emulator.memory[1000] = 12;
    emu_result_t result = emu_8086_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(4 == g_emulator.registers.bx);  // 0b0101 & 0b1100 = 0b0100
}

Test(emu__I_AND__tests, and__direct_access__m8_2, .init = emu_and_default_setup) {
    uint8_t input[] = {0x20, 0x1e, 0xe8, 0x03};  // "and [1000], bl"
    g_emulator.registers.bx = 5;                 // 0b0101
    g_emulator.memory[1000] = 12;
    emu_result_t result = emu_8086_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    LOGD("bx: %d, [1000]: %d", g_emulator.registers.bx, g_emulator.memory[1000]);
    cr_assert(4 == g_emulator.memory[1000]);  // 0b0101 & 0b1100 = 0b0100
}

Test(emu__I_AND__tests, and__direct_access__m16, .init = emu_and_default_setup) {
    uint8_t input[] = {0x23, 0x1e, 0xe8, 0x03};  // "and bx, [1000]"
    g_emulator.registers.bx = 0xFF80;
    g_emulator.memory[1000] = 0x88;
    g_emulator.memory[1001] = 0x88;
    emu_result_t result = emu_8086_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(0x8880 == g_emulator.registers.bx);
}

Test(emu__I_AND__tests, and__direct_access__m16_2, .init = emu_and_default_setup) {
    uint8_t input[] = {0x21, 0x1e, 0xe8, 0x03};  // "and [1000], bx"
    g_emulator.registers.bx = 0x8888;
    g_emulator.memory[1000] = 0xFF;
    g_emulator.memory[1001] = 0x80;
    emu_result_t result = emu_8086_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(0x88 == g_emulator.memory[1000]);
    cr_assert(0x80 == g_emulator.memory[1001]);
}

// MARK: I_AND_IMMEDIATE
// I_AND_IMMEDIATE - register
Test(emu__I_AND_IMMEDIATE__tests, and__register__m8, .init = emu_and_default_setup) {
    uint8_t input[] = {0x80, 0xe1, 0x0c};  // "and cl, 12" - 0b10000000 0b11100001 ...
    g_emulator.registers.cx = 5;           // 0b0101
    emu_result_t result = emu_8086_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(4 == g_emulator.registers.cx);  // 0b0101 & 0b1100 = 0b0100
}

Test(emu__I_AND_IMMEDIATE__tests, and__register__m16, .init = emu_and_default_setup) {
    uint8_t input[] = {0x83, 0xe1, 0x0c};  // "and cx, 12" - 0b10000011 0b11100001 ...
    g_emulator.registers.cx = 5;           // 0b0101
    emu_result_t result = emu_8086_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(4 == g_emulator.registers.cx);  // 0b0101 & 0b1100 = 0b0100
}

Test(emu__I_AND_IMMEDIATE__tests, and__register__m16_2, .init = emu_and_default_setup) {
    uint8_t input[] = {0x81, 0xe1, 0xf1, 0xf0};  // "and cx, 0xF0F1"
    g_emulator.registers.cx = 0x1111;
    emu_result_t result = emu_8086_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(0x1011 == g_emulator.registers.cx);  // 0xF0F1 & 0x1111 = 0x1011
}

Test(emu__I_AND_IMMEDIATE__tests, and__register__m16_3, .init = emu_and_default_setup) {
    // NOTE: 3 bytes instead of 4, 0xFFF0 is just 0xF0 because of "signed bit extension".
    uint8_t input[] = {0x83, 0xe1, 0xf0};  // "and cx, 0xFFF0"
    g_emulator.registers.cx = 0x1111;
    emu_result_t result = emu_8086_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    printf("cx: %x\n", g_emulator.registers.cx);
    cr_assert(0x1110 == g_emulator.registers.cx);  // 0xFFF0 & 0x1111 = 0x1110
}

Test(
    emu__I_AND_IMMEDIATE__tests,
    and__register__m16_signed_extension,
    .init = emu_and_default_setup
) {
    // NOTE: 3 bytes instead of 4, 0xFFF0 is just 0xF0 because of "signed bit extension".
    uint8_t input[] = {0x83, 0xe1, 0x80};  // "and cx, 0xFF80"
    g_emulator.registers.cx = 0x8888;
    emu_result_t result = emu_8086_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    printf("cx: %x\n", g_emulator.registers.cx);
    cr_assert(0x8880 == g_emulator.registers.cx);  // 0xFF80 & 0x8888 = 0x8880
}

// I_AND_IMMEDIATE - direct access
Test(emu__I_AND_IMMEDIATE__tests, and__direct_access__m8, .init = emu_and_default_setup) {
    uint8_t input[] = {0x80, 0x26, 0xe8, 0x03, 0x0c};  // "and byte [1000], 12" - 0b10000000 ...
    g_emulator.memory[1000] = 5;                       // 0b0101
    emu_result_t result = emu_8086_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(4 == g_emulator.memory[1000]);  // 0b0101 & 0b1100 = 0b0100
}

// I_AND_IMMEDIATE - memory
// I_AND_IMMEDIATE - memory + displacement

// MARK: I_AND_IMMEDIATE_TO_AX
Test(emu__I_AND_IMMEDIATE_TO_AX__tests, and__register__m8, .init = emu_and_default_setup) {
    uint8_t input[] = {0x24, 0x0c};  // "and al, 12"
    g_emulator.registers.ax = 5;     // 0b0101
    emu_result_t result = emu_8086_emulate_chunk(&g_emulator, input, sizeof(input));
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(4 == g_emulator.registers.ax);  // 0b0101 & 0b1100 = 0b0100
}
