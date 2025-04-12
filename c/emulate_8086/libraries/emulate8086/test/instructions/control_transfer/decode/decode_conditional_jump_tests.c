/**
 * Decode tests for conditional jump instructions.
 */
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"

#include "libraries/emulate8086/include/emulate8086.h"

//
// MARK: JNZ/JNE
//

static emulator_t g_decoder;

void decode_conditional_jump_default_setup(void) {
    memset(&g_decoder, 0, sizeof(emulator_t));
    emu_init(&g_decoder);
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
