/**
 * Decode tests for the "not" instruction.
 */
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "8086/instruction_tags_8086.h"

#include "8086/emulate_8086.h"

//
// MARK: NOT
//

static emulator_8086_t g_emulator;

void decode_not_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_8086_t));
    emu_8086_init(&g_emulator);
}

// MARK: 1. I_NOT
Test(decode__I_NOT__tests, not_1, .init = decode_not_default_setup)
{
    char* expected = "not ax\n";
    uint8_t input[] = { 0xf7, 0xd0 }; // 0b11110111 0b
    char output[32] = { 0x00 };
    cr_assert(SUCCESS == emu_8086_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}
