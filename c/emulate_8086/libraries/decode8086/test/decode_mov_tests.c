/**
 * Decode tests for the "mov" instruction.
 *
 * NOTE: when building test data, use "xxd", the "-i" argument adds syntax for c arrays
 * to the output. Reminder to myself that "hexdump" outputs 16 bit words and will cause
 * every pair of bytes to be flipped on little endian machines.
 * `xxd -i {my_assembled_file}`
 *
 * Remeber to also add "bits 16" to the top of any assembly files passed to nasm to get
 * the older x86 era instructions. ex:
 * ```asm
 * ; my_assembly.asm
 * ; compile to machine code with: `nasm ./my_assemby.asm`
 * bits 16
 * mov ch, ah
 * ```
 */
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"

#include "libraries/decode8086/include/decode8086.h"

//
// MARK: MOV
//

// Use of this global "g_decoder" is to try and reduce the amount of code per test. It's reset
// after each test and has a large default instructions buffer.
static instruction_t g_instructions[4096];
static int g_instructions_size = 4096;
static decoder_t g_decoder;

void default_setup(void) {
    memset(&g_decoder, 0, sizeof(decoder_t));
    dcd_init(&g_decoder, g_instructions, g_instructions_size);
}

// MARK: 1. I_MOVE
Test(decode__I_MOVE__tests,
     mov1, .init = default_setup)
{
    char* expected = "mov cx, bx\n";
    uint8_t input[] = { 0x89, 0xd9 };
    cr_assert(SUCCESS == dcd_decode_chunk(&g_decoder, input, sizeof(input)));
    cr_assert(1 == g_decoder.instructions_count);
    char output[32] = { 0x00 };
    dcd_write_all_assembly(g_decoder.instructions, g_decoder.instructions_count, output, sizeof(output));
    cr_assert(strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

Test(decode__I_MOVE__tests,
     mov2, .init = default_setup)
{
    char* expected = "mov ch, ah\n";
    uint8_t input[] = { 0x88, 0xe5 };
    cr_assert(SUCCESS == dcd_decode_chunk(&g_decoder, input, sizeof(input)));
    cr_assert(1 == g_decoder.instructions_count);
    char output[16] = { 0x00 };
    dcd_write_all_assembly(g_decoder.instructions, g_decoder.instructions_count, output, sizeof(output));
    cr_assert(strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

Test(decode__I_MOVE__tests,
     mov3, .init = default_setup)
{
    char* expected = "mov si, bx\n";
    uint8_t input[] = { 0x89, 0xde };
    cr_assert(SUCCESS == dcd_decode_chunk(&g_decoder, input, sizeof(input)));
    cr_assert(1 == g_decoder.instructions_count);
    char output[16] = { 0x00 };
    dcd_write_all_assembly(g_decoder.instructions, g_decoder.instructions_count, output, sizeof(output));
    cr_assert(strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

Test(decode__I_MOVE__tests,
     bulk_mov, .init = default_setup)
{
    char* expected = "mov cx, bx\n\
mov ch, ah\n\
mov dx, bx\n\
mov si, bx\n\
mov bx, di\n\
mov al, cl\n\
mov ch, ch\n\
mov bx, ax\n\
mov bx, si\n\
mov sp, di\n\
mov bp, ax\n";
    uint8_t input[] = {
        0x89, 0xd9, 0x88, 0xe5, 0x89, 0xda, 0x89, 0xde, 0x89, 0xfb, 0x88, 0xc8,
        0x88, 0xed, 0x89, 0xc3, 0x89, 0xf3, 0x89, 0xfc, 0x89, 0xc5
    };
    cr_assert(SUCCESS == dcd_decode_chunk(&g_decoder, input, sizeof(input)));
    cr_assert(11 == g_decoder.instructions_count);
    char output[256] = { 0x00 };
    dcd_write_all_assembly(g_decoder.instructions, g_decoder.instructions_count, output, sizeof(output));
    cr_assert(strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

Test(decode__I_MOVE__tests,
     mov4, .init = default_setup)
{
    char* expected = "mov bp, [5]\n";
    uint8_t input[] = { 0x8b, 0x2e, 0x05, 0x00 };
    cr_assert(SUCCESS == dcd_decode_chunk(&g_decoder, input, sizeof(input)));
    cr_assert(1 == g_decoder.instructions_count);
    char output[16] = { 0x00 };
    dcd_write_all_assembly(g_decoder.instructions, g_decoder.instructions_count, output, sizeof(output));
    cr_assert(strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

// MARK: 2. I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY
// TODO

// MARK: 3. I_MOVE_IMMEDIATE_TO_REGISTER
Test(decode__I_MOVE_IMMEDIATE_TO_REGISTER__tests,
     mov1, .init = default_setup)
{
    char* expected = "mov cl, 12\n";
    uint8_t input[] = { 0xb1, 0x0c };
    cr_assert(SUCCESS == dcd_decode_chunk(&g_decoder, input, sizeof(input)));
    cr_assert(1 == g_decoder.instructions_count);
    char output[16] = { 0x00 };
    dcd_write_all_assembly(g_decoder.instructions, g_decoder.instructions_count, output, sizeof(output));
    cr_assert(strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

Test(decode__I_MOVE_IMMEDIATE_TO_REGISTER__tests,
     mov2, .init = default_setup)
{
    char* expected = "mov ch, 244\n"; // mov cl, -12 ; cpu doesn't care about signedness until you do an arithmetic operation
    uint8_t input[] = { 0xb5, 0xf4 };
    cr_assert(SUCCESS == dcd_decode_chunk(&g_decoder, input, sizeof(input)));
    cr_assert(1 == g_decoder.instructions_count);
    char output[16] = { 0x00 };
    dcd_write_all_assembly(g_decoder.instructions, g_decoder.instructions_count, output, sizeof(output));
    cr_assert(strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

// MARK: 4. I_MOVE_MEMORY_TO_ACCUMULATOR
Test(decode__I_MOVE_MEMORY_TO_ACCUMULATOR__tests,
     mov1, .init = default_setup)
{
    char* expected = "mov ax, [2555]\n";
    uint8_t input[] = { 0xa1, 0xfb, 0x09 };
    cr_assert(SUCCESS == dcd_decode_chunk(&g_decoder, input, sizeof(input)));
    cr_assert(1 == g_decoder.instructions_count);
    char output[16] = { 0x00 };
    dcd_write_all_assembly(g_decoder.instructions, g_decoder.instructions_count, output, sizeof(output));
    cr_assert(strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

// MARK: 5. I_MOVE_ACCUMULATOR_TO_MEMORY
Test(decode__I_MOVE_ACCUMULATOR_TO_MEMORY__tests,
     mov1, .init = default_setup)
{
    char* expected = "mov [2554], ax\n";
    uint8_t input[] = { 0xa3, 0xfa, 0x09 };
    cr_assert(SUCCESS == dcd_decode_chunk(&g_decoder, input, sizeof(input)));
    cr_assert(1 == g_decoder.instructions_count);
    char output[16] = { 0x00 };
    dcd_write_all_assembly(g_decoder.instructions, g_decoder.instructions_count, output, sizeof(output));
    cr_assert(strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

// MARK: 6. I_MOVE_REGISTER_OR_MEMORY_TO_SEGMENT_REGISTER
// TODO
// MARK: 7. I_MOVE_SEGMENT_REGISTER_TO_REGISTER_OR_MEMORY
// TODO

// MARK: MISC

Test(decode__mov_misc__tests,
     bulk_mov, .init = default_setup)
{
    char* expected = "mov si, bx\n\
mov dh, al\n\
mov cl, 12\n\
mov ch, 244\n\
mov cx, 12\n\
mov cx, 65524\n\
mov dx, 3948\n\
mov dx, 61588\n\
mov al, [bx + si]\n\
mov bx, [bp + di]\n\
mov dx, [bp]\n\
mov ah, [bx + si + 4]\n\
mov al, [bx + si + 4999]\n\
mov [bx + di], cx\n\
mov [bp + si], cl\n\
mov [bp], ch\n";
    uint8_t input[] = {
        0x89, 0xde, 0x88, 0xc6, 0xb1, 0x0c, 0xb5, 0xf4, 0xb9, 0x0c, 0x00, 0xb9,
        0xf4, 0xff, 0xba, 0x6c, 0x0f, 0xba, 0x94, 0xf0, 0x8a, 0x00, 0x8b, 0x1b,
        0x8b, 0x56, 0x00, 0x8a, 0x60, 0x04, 0x8a, 0x80, 0x87, 0x13, 0x89, 0x09,
        0x88, 0x0a, 0x88, 0x6e, 0x00
    };
    cr_assert(SUCCESS == dcd_decode_chunk(&g_decoder, input, sizeof(input)));
    cr_assert(16 == g_decoder.instructions_count);
    char output[512] = { 0x00 };
    dcd_write_all_assembly(g_decoder.instructions, g_decoder.instructions_count, output, sizeof(output));
    cr_assert(strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

// Test(decode__mov_misc__tests,
//      bulk_mov2, .init = default_setup)
// {
//     char* expected = "mov ax, [bx + di - 37]\n\
// mov [si - 300], cx\n\
// mov dx, [bx - 32]\n\
// mov [bp + di], byte 7\n\
// mov [di + 901], word 347\n\
// mov bp, [5]\n\
// mov bx, [3458]\n\
// mov ax, [2555]\n\
// mov ax, [16]\n\
// mov [2554], ax\n\
// mov [15], ax\n";
//     uint8_t input[] = {
//         0x8b, 0x41, 0xdb, 0x89, 0x8c, 0xd4, 0xfe, 0x8b, 0x57, 0xe0, 0xc6, 0x03,
//         0x07, 0xc7, 0x85, 0x85, 0x03, 0x5b, 0x01, 0x8b, 0x2e, 0x05, 0x00, 0x8b,
//         0x1e, 0x82, 0x0d, 0xa1, 0xfb, 0x09, 0xa1, 0x10, 0x00, 0xa3, 0xfa, 0x09,
//         0xa3, 0x0f, 0x00
//     };
//     cr_assert(SUCCESS == dcd_decode_chunk(&g_decoder, input, sizeof(input)));
//     cr_assert(11 == g_decoder.instructions_count);
//     uint8_t output[512] = { 0x00 };
//     dcd_write_all_assembly(g_decoder.instructions, g_decoder.instructions_count, output, sizeof(output));
//     cr_assert(strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
// }
