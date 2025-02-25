
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"

#include "libraries/emulate8086/include/emulate8086.h"

//
// MARK: PUSH
//

// Use of this global "g_decoder" is to try and reduce the amount of code per test. It's reset
// after each test and has a large default instructions buffer.
static emulator_t g_decoder;

void decode_push_default_setup(void) {
    memset(&g_decoder, 0, sizeof(emulator_t));
    emu_init(&g_decoder);
}

// MARK: 2. I_PUSH_REGISTER Tests
Test(decode__I_PUSH_REGISTER__tests, push_register_1, .init = decode_push_default_setup)
{
    char* expected = "push cx\n";
    uint8_t input[] = { 0x51 }; // 0b01010001
    char output[32] = { 0x00 };
    cr_assert(SUCCESS == emu_decode_chunk(
        &g_decoder, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_decoder.instructions_count,
        "expected:\n'%d'\n\nactual:\n'%d'\n", 1, g_decoder.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}
