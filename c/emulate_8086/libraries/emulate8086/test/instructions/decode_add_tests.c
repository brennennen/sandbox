
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"

#include "libraries/emulate8086/include/decode8086.h"

//
// MARK: ADD
//

// Use of this global "g_decoder" is to try and reduce the amount of code per test. It's reset
// after each test and has a large default instructions buffer.
//static instruction_t g_instructions[4096];
//static int g_instructions_size = 4096;
static emulator_t g_decoder;

void add_default_setup(void) {
    memset(&g_decoder, 0, sizeof(emulator_t));
    emu_init(&g_decoder);
}

// MARK: 1. I_ADD Tests
Test(decode__I_ADD__tests,
     add_1, .init = add_default_setup)
{
    char* expected = "add cx, bx\n";
    uint8_t input[] = { 0x01, 0xd9 };
    char output[32] = { 0x00 };
    cr_assert(SUCCESS == emu_decode_chunk(
        &g_decoder, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_decoder.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

// MARK: 2. I_ADD_IMMEDIATE Tests
Test(decode__I_ADD_IMMEDIATE__tests,
     add_immediate_1, .init = add_default_setup)
{
    char* expected = "add cx, 8\n";
    uint8_t input[] = { 0x83, 0xc1, 0x08 }; // 0b10000011 11000001 00001000
    char output[32] = { 0x00 };
    cr_assert(SUCCESS == emu_decode_chunk(
        &g_decoder, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_decoder.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}


// MARK: 3. I_ADD Tests

// MARK: MISC Tests
// Test(decode__add_misc__tests,
//      bulk_add_tests1, .init = add_default_setup)
// {
//     char* expected = "add bx, [bx+si]\n\
// add bx, [bp]\n\
// add si, 2\n\
// add bp, 2\n\
// add cx, 8\n\
// add bx, [bp + 0]\n\
// add cx, [bx + 2]\n\
// add bh, [bp + si + 4]\n\
// add di, [bp + di + 6]\n\
// add [bx+si], bx\n\
// add [bp], bx\n\
// add [bp + 0], bx\n\
// add [bx + 2], cx\n\
// add [bp + si + 4], bh\n\
// add [bp + di + 6], di\n\
// add byte [bx], 34\n\
// add word [bp + si + 1000], 29\n\
// add ax, [bp]\n\
// add al, [bx + si]\n\
// add ax, bx\n\
// add al, ah\n\
// add ax, 1000\n\
// add al, -30\n\
// add al, 9\n";
//     uint8_t input[] = {
//         0x03, 0x18, 0x03, 0x5e, 0x00, 0x83, 0xc6, 0x02, 0x83, 0xc5, 0x02, 0x83,
//         0xc1, 0x08, 0x03, 0x5e, 0x00, 0x03, 0x4f, 0x02, 0x02, 0x7a, 0x04, 0x03,
//         0x7b, 0x06, 0x01, 0x18, 0x01, 0x5e, 0x00, 0x01, 0x5e, 0x00, 0x01, 0x4f,
//         0x02, 0x00, 0x7a, 0x04, 0x01, 0x7b, 0x06, 0x80, 0x07, 0x22, 0x83, 0x82,
//         0xe8, 0x03, 0x1d, 0x03, 0x46, 0x00, 0x02, 0x00, 0x01, 0xd8, 0x00, 0xe0,
//         0x05, 0xe8, 0x03, 0x04, 0xe2, 0x04, 0x09
//     };
//     cr_assert(SUCCESS == emu_decode_chunk(&g_decoder, input, sizeof(input), output, sizeof(output)));
//     cr_assert(1 == g_decoder.instructions_count);
//     uint8_t output[32] = { 0x00 };
//     dcd_write_all_assembly(g_decoder.instructions, g_decoder.instructions_count, output, sizeof(output));
//     cr_assert(strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
// }
