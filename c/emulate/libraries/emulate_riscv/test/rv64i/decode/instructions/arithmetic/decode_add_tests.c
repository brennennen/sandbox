
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64i/emulate_rv64i.h"

// Use of this global "g_emulator" is to try and reduce the amount of code per test. It's reset
// after each test and has a large default instructions buffer.
static emulator_rv64i_t g_emulator;

void emu_rv64i_decode_add_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64i_t));
    emu_rv64i_init(&g_emulator);
}

// MARK: I_RV64I_ADD_IMMEDIATE Tests
Test(emu_rv64i_decode__I_ADD_IMMEDIATE__tests, add_immediate_1, .init = emu_rv64i_decode_add_default_setup)
{
    char* expected = "addi t0, t1, 5\n";
    uint8_t input[] = { 0x00, 0x53, 0x02, 0x93 }; // 00530293
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64i_decode_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}
