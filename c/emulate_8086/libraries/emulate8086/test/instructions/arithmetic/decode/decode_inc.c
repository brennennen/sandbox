
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"

#include "libraries/emulate8086/include/emulate8086.h"

//
// MARK: INC
//

// Use of this global "g_decoder" is to try and reduce the amount of code per test. It's reset
// after each test and has a large default instructions buffer.
static emulator_t g_decoder;

void decode_inc_default_setup(void) {
    memset(&g_decoder, 0, sizeof(emulator_t));
    emu_init(&g_decoder);
}

// MARK: 1. I_INC Tests

// MARK: 2. I_INC_REGISTER Tests
Test(decode__I_INC_REGISTER__tests, inc_register_1, .init = decode_inc_default_setup)
{
    char* expected = "inc cx\n";
    uint8_t input[] = { 0x41 };
    char output[32] = { 0x00 };
    cr_assert(SUCCESS == emu_decode_chunk(
        &g_decoder, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_decoder.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}
