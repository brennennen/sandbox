
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "a64/emulate_a64.h"

// Use of this global "g_emulator" is to try and reduce the amount of code per test. It's reset
// after each test and has a large default instructions buffer.
static emulator_a64_t g_emulator;

void emu_a64_decode_add_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_a64_t));
    emu_a64_init(&g_emulator);
}

// MARK: I_ADD_IMMEDIATE Tests
Test(
    emu_a64_decode__I_ADD_IMMEDIATE__tests,
    add_immediate_1,
    .init = emu_a64_decode_add_default_setup
) {
    char* expected = "add x1, x2, #5\n";
    uint8_t input[] = {0x91, 0x00, 0x14, 0x41};  // 91001441
    char output[32] = {'\0'};
    cr_assert(
        SUCCESS == emu_a64_decode_chunk(&g_emulator, input, sizeof(input), output, sizeof(output))
    );
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(
        strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n",
        expected, output
    );
}
