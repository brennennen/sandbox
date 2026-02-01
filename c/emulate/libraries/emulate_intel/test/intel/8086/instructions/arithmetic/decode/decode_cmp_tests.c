
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "8086/instruction_tags_8086.h"

#include "8086/emulate_8086.h"

//
// MARK: CMP
//

// Use of this global "g_emulator" is to try and reduce the amount of code per test. It's reset
// after each test and has a large default instructions buffer.
static emulator_8086_t g_emulator;

void cmp_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_8086_t));
    emu_8086_init(&g_emulator);
}

// MARK: 1. I_CMP Tests
Test(decode__I_CMP__tests, cmp_1, .init = cmp_default_setup) {
    char* expected = "cmp si, dx\n";
    uint8_t input[] = {0x39, 0xd6};
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
