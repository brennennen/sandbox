
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"

#include "libraries/decode8086/include/decode8086.h"

//
// MARK: ADD
//

// Use of this global "g_decoder" is to try and reduce the amount of code per test. It's reset
// after each test and has a large default instructions buffer.
static instruction_t g_instructions[4096];
static int g_instructions_size = 4096;
static decoder_t g_decoder;

void add_default_setup(void) {
    memset(&g_decoder, 0, sizeof(decoder_t));
    dcd_init(&g_decoder, g_instructions, g_instructions_size);
}

// MARK: 1. I_ADD_REGISTER_OR_MEMORY_WITH_REGISTER_TO_EITHER
Test(decode__I_ADD_REGISTER_OR_MEMORY_WITH_REGISTER_TO_EITHER__tests,
     mov1, .init = add_default_setup)
{
    char* expected = "add cx, bx\n";
    uint8_t input[] = { 0x01, 0xd9 };
    cr_assert(SUCCESS == dcd_decode_chunk(&g_decoder, input, sizeof(input)));
    cr_assert(1 == g_decoder.instructions_count);
    uint8_t output[32] = { 0x00 };
    dcd_write_all_assembly(g_decoder.instructions, g_decoder.instructions_count, output, sizeof(output));
    cr_assert(strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}
