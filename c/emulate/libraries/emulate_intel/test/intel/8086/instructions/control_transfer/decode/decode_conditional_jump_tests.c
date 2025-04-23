/**
 * Decode tests for conditional jump instructions.
 */
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"

#include "libraries/emulate_intel/include/emulate.h"

//
// MARK: JNZ/JNE
//

static emulator_t g_decoder;

void decode_conditional_jump_default_setup(void) {
    memset(&g_decoder, 0, sizeof(emulator_t));
    emu_init(&g_decoder);
}

Test(decode__I_JE__tests, je_1, .init = decode_conditional_jump_default_setup)
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

Test(decode__conditional_jumps_bulk__tests, bulk_1, .init = decode_conditional_jump_default_setup)
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

Test(decode__conditional_jump__tests, jnz, .init = decode_conditional_jump_default_setup)
{
// loop_start:
// jnz loop_start
    char* expected = "jne $+0\n";
    uint8_t input[] = { 0x75, 0xfe };
    char output[256] = { 0x00 };
    cr_assert(SUCCESS == emu_decode_chunk(
        &g_decoder, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_decoder.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

Test(decode__conditional_jump__tests, jnz2, .init = decode_conditional_jump_default_setup)
{
// mov cx, 3
// mov bx, 1000
// loop_start:
// add bx, 10
// sub cx, 1
// jnz loop_start
    char* expected = "mov cx, 3\n\
mov bx, 1000\n\
add bx, 10\n\
sub cx, 1\n\
jne $-6\n";
    uint8_t input[] = {
        0xb9, 0x03, 0x00, 0xbb, 0xe8, 0x03, 0x83, 0xc3, 0x0a, 0x83, 0xe9, 0x01,
        0x75, 0xf8
    };
    char output[256] = { 0x00 };
    cr_assert(SUCCESS == emu_decode_chunk(
        &g_decoder, input, sizeof(input), output, sizeof(output)));
    cr_assert(5 == g_decoder.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

Test(decode__conditional_jump__tests, jnz3, .init = decode_conditional_jump_default_setup)
{
// mov dx, 6
// mov bp, 1000
//
// mov si, 0
// init_loop_start:
//     mov word [bp + si], si
//     add si, 2
//     cmp si, dx
//     jnz init_loop_start
//
// mov bx, 0
// mov si, 0
// add_loop_start:
//     mov cx, word [bp + si]
//     add bx, cx
//     add si, 2
//     cmp si, dx
//     jnz add_loop_start
    char* expected = "mov dx, 6\n\
mov bp, 1000\n\
mov si, 0\n\
mov [bp + si], si\n\
add si, 2\n\
cmp si, dx\n\
jne $-7\n\
mov bx, 0\n\
mov si, 0\n\
mov cx, [bp + si]\n\
add bx, cx\n\
add si, 2\n\
cmp si, dx\n\
jne $-9\n\
";
    uint8_t input[] = {
        0xba, 0x06, 0x00, 0xbd, 0xe8, 0x03, 0xbe, 0x00, 0x00, 0x89, 0x32, 0x83,
        0xc6, 0x02, 0x39, 0xd6, 0x75, 0xf7, 0xbb, 0x00, 0x00, 0xbe, 0x00, 0x00,
        0x8b, 0x0a, 0x01, 0xcb, 0x83, 0xc6, 0x02, 0x39, 0xd6, 0x75, 0xf5
    };
    char output[256] = { 0x00 };
    cr_assert(SUCCESS == emu_decode_chunk(
        &g_decoder, input, sizeof(input), output, sizeof(output)));
    cr_assert(14 == g_decoder.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}
