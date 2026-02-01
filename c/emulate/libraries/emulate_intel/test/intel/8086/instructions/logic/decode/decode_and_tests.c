/**
 * Decode tests for the "and" instruction.
 */
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "8086/emulate_8086.h"
#include "8086/instruction_tags_8086.h"

static emulator_8086_t g_emulator;

void decode_and_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_8086_t));
    emu_8086_init(&g_emulator);
}

// MARK: 1. I_AND
Test(decode__I_AND__tests, and_1, .init = decode_and_default_setup) {
    char* expected = "and bx, cx\n";
    uint8_t input[] = {0x21, 0xcb};  // 0b00100001 0b11001011
    char output[32] = {0x00};
    cr_assert(
        SUCCESS
        == emu_8086_disassemble_chunk(&g_emulator, input, sizeof(input), output, sizeof(output))
    );
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(
        strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n",
        expected, output
    );
}

// MARK: 2. I_AND_IMMEDIATE
Test(decode__I_AND_IMMEDIATE__tests, and_immediate_1, .init = decode_and_default_setup) {
    char* expected = "and bx, 16\n";
    uint8_t input[] = {0x83, 0xe3, 0x10};  // 0b10000011 0b11100011 0b00010000
    char output[32] = {0x00};
    cr_assert(
        SUCCESS
        == emu_8086_disassemble_chunk(&g_emulator, input, sizeof(input), output, sizeof(output))
    );
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(
        strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n",
        expected, output
    );
}

Test(decode__I_AND_IMMEDIATE__tests, and_immediate_2, .init = decode_and_default_setup) {
    char* expected = "and byte [bx + 1000], 16\n";
    uint8_t input[] = {0x80, 0xa7, 0xe8, 0x03, 0x10};  // 0b10000000 0b10100111 ...
    char output[32] = {0x00};
    cr_assert(
        SUCCESS
        == emu_8086_disassemble_chunk(&g_emulator, input, sizeof(input), output, sizeof(output))
    );
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(
        strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n",
        expected, output
    );
}

Test(
    decode__I_AND_IMMEDIATE__tests,
    and_immediate_3_signed_extension,
    .init = decode_and_default_setup
) {
    char* expected = "and cx, 65408\n";  // "and cx, 0xFF80\n";
    uint8_t input[] = {0x83, 0xe1, 0x80};
    char output[32] = {0x00};
    cr_assert(
        SUCCESS
        == emu_8086_disassemble_chunk(&g_emulator, input, sizeof(input), output, sizeof(output))
    );
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(
        strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n",
        expected, output
    );
}

// MARK: I_AND_IMMEDIATE_TO_AX
Test(
    decode__I_AND_IMMEDIATE_TO_AX__tests,
    and_immediate_to_ax_1,
    .init = decode_and_default_setup
) {
    char* expected = "and al, 16\n";
    uint8_t input[] = {0x24, 0x10};  // 0b00100100 (immediate)
    char output[32] = {0x00};
    cr_assert(
        SUCCESS
        == emu_8086_disassemble_chunk(&g_emulator, input, sizeof(input), output, sizeof(output))
    );
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(
        strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n",
        expected, output
    );
}

Test(
    decode__I_AND_IMMEDIATE_TO_AX__tests,
    and_immediate_to_ax_2,
    .init = decode_and_default_setup
) {
    char* expected = "and ax, 4113\n";
    uint8_t input[] = {0x25, 0x11, 0x10};  // 0b00100100 (immediate)
    char output[32] = {0x00};
    cr_assert(
        SUCCESS
        == emu_8086_disassemble_chunk(&g_emulator, input, sizeof(input), output, sizeof(output))
    );
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(
        strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n",
        expected, output
    );
}
