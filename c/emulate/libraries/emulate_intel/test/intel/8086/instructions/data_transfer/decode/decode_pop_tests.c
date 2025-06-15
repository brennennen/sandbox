
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "8086/instruction_tags_8086.h"

#include "8086/emulate_8086.h"

//
// MARK: POP
//

// Use of this global "g_emulator" is to try and reduce the amount of code per test. It's reset
// after each test and has a large default instructions buffer.
static emulator_8086_t g_emulator;

void decode_pop_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_8086_t));
    emu_8086_init(&g_emulator);
}

// MARK: 2. I_POP_REGISTER Tests
Test(decode__I_POP_REGISTER__tests, pop_register_1, .init = decode_pop_default_setup)
{
    char* expected = "pop cx\n";
    uint8_t input[] = { 0x59 }; // 0b01011001
    char output[32] = { 0x00 };
    cr_assert(SUCCESS == emu_8086_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count,
        "expected:\n'%d'\n\nactual:\n'%d'\n", 1, g_emulator.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}
