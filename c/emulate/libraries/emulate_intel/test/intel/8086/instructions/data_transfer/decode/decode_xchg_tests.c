
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"

#include "libraries/emulate_intel/include/emulate.h"

//
// MARK: XCHG
//

// Use of this global "g_decoder" is to try and reduce the amount of code per test. It's reset
// after each test and has a large default instructions buffer.
static emulator_t g_decoder;

void decode_xchg_default_setup(void) {
    memset(&g_decoder, 0, sizeof(emulator_t));
    emu_init(&g_decoder);
}

// MARK: 1. I_EXCHANGE Tests
Test(decode__I_EXCHANGE__tests, xchg_1, .init = decode_xchg_default_setup)
{
    char* expected = "xchg bx, cx\n";
    uint8_t input[] = { 0x87, 0xcb }; // 0b10000111 0b11001011
    char output[32] = { 0x00 };
    cr_assert(SUCCESS == emu_decode_chunk(
        &g_decoder, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_decoder.instructions_count,
        "expected:\n'%d'\n\nactual:\n'%d'\n", 1, g_decoder.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

// MARK: 2. I_EXCHANGE_AX Tests
Test(decode__I_EXCHANGE_AX__tests, xchg_ax_1, .init = decode_xchg_default_setup)
{
    char* expected = "xchg cx, ax\n";
    uint8_t input[] = { 0x91 }; // 0b10010001
    char output[32] = { 0x00 };
    cr_assert(SUCCESS == emu_decode_chunk(
        &g_decoder, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_decoder.instructions_count,
        "expected:\n'%d'\n\nactual:\n'%d'\n", 1, g_decoder.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}
