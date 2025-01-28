
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"

#include "libraries/decode8086/include/decode8086.h"

//
// MARK: SUB
//

// Use of this global "g_decoder" is to try and reduce the amount of code per test. It's reset
// after each test and has a large default instructions buffer.
static instruction_t g_instructions2[4096];
static int g_instructions_size2 = 4096;
static decoder_t g_decoder2;

void sub_default_setup(void) {
    memset(&g_decoder2, 0, sizeof(decoder_t));
    dcd_init(&g_decoder2, g_instructions2, g_instructions_size2);
}

// MARK: 1. I_SUB Tests
Test(decode__I_SUB__tests,
     sub_1, .init = sub_default_setup)
{
    char* expected = "sub cx, bx\n";
    uint8_t input[] = { 0x29, 0xd9 }; // 0b00101001 0b11011001
    cr_assert(SUCCESS == dcd_decode_chunk(&g_decoder2, input, sizeof(input)));
    cr_assert(1 == g_decoder2.instructions_count);
    uint8_t output[32] = { 0x00 };
    dcd_write_all_assembly(g_decoder2.instructions, g_decoder2.instructions_count, output, sizeof(output));
    cr_assert(strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);

}