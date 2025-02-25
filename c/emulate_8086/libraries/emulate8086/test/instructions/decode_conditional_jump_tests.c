
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"

#include "libraries/emulate8086/include/emulate8086.h"

// Use of this global "g_decoder" is to try and reduce the amount of code per test. It's reset
// after each test and has a large default instructions buffer.
static emulator_t g_decoder;

void cond_jump_default_setup(void) {
    memset(&g_decoder, 0, sizeof(emulator_t));
    emu_init(&g_decoder);
}

Test(decode__I_JE__tests, je_1, .init = cond_jump_default_setup)
{
    /* expected (using relative addressing to avoid needing to manage labels):
label
je label
    */
    char* expected = "je $+0\n";
    uint8_t input[] = { 0x74, 0xfe }; // 0b00101001 0b11011001
    char output[32] = { 0x00 };
    cr_assert(SUCCESS == emu_decode_chunk(
        &g_decoder, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_decoder.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

Test(decode__conditional_jumps_bulk__tests,
     bulk_1, .init = cond_jump_default_setup)
{
    /* expected (using relative addressing to avoid needing to manage labels):
test_label0:
jnz test_label1
jnz test_label0
test_label1:
jnz test_label0
jnz test_label1

label:
je label
jl label
jle label
jb label
jbe label
jp label
jo label
js label
jne label
jnl label
jg label
jnb label
ja label
jnp label
jno label
jns label
loop label
loopz label
loopnz label
jcxz label
    */
    char* expected = "jne $+4\n\
jne $-2\n\
jne $-4\n\
jne $-2\n\
je $+0\n\
jl $-2\n\
jle $-4\n\
jb $-6\n\
jbe $-8\n\
jp $-10\n\
jo $-12\n\
js $-14\n\
jne $-16\n\
jnl $-18\n\
jnle $-20\n\
jnb $-22\n\
jnbe $-24\n\
jnp $-26\n\
jno $-28\n\
jns $-30\n\
loop $-32\n\
loopz $-34\n\
loopnz $-36\n\
jcxz $-38\n\
";
    //printf("g_decoder.instructions_cap: %d\n", g_decoder.instructions_capacity);
    uint8_t input[] = {
        0x75, 0x02, 0x75, 0xfc, 0x75, 0xfa, 0x75, 0xfc, 0x74, 0xfe, 0x7c, 0xfc,
        0x7e, 0xfa, 0x72, 0xf8, 0x76, 0xf6, 0x7a, 0xf4, 0x70, 0xf2, 0x78, 0xf0,
        0x75, 0xee, 0x7d, 0xec, 0x7f, 0xea, 0x73, 0xe8, 0x77, 0xe6, 0x7b, 0xe4,
        0x71, 0xe2, 0x79, 0xe0, 0xe2, 0xde, 0xe1, 0xdc, 0xe0, 0xda, 0xe3, 0xd8
    };
    char output[512] = { 0x00 };
    cr_assert(SUCCESS == emu_decode_chunk(
        &g_decoder, input, sizeof(input), output, sizeof(output)));
    cr_assert(24 == g_decoder.instructions_count,
        "expected: '%d', actual:'%d'\n", 24, g_decoder.instructions_count);

    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}
