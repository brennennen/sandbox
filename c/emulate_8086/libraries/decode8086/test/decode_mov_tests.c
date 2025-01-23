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

// MARK: 1. I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY
Test(decode__I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY__tests, 
     mov1, .init = default_setup) 
{
    char* expected = "mov cx, bx\n";
    uint8_t input[] = { 0x89, 0xd9 };
    cr_assert(SUCCESS == dcd_decode_chunk(&g_decoder, input, sizeof(input)));
    cr_assert(1 == g_decoder.instructions_count);
    uint8_t output[32] = { 0x00 };
    dcd_write_all_assembly(g_decoder.instructions, g_decoder.instructions_count, output, sizeof(output));
    cr_assert(strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

Test(decode__I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY__tests, 
     mov2, .init = default_setup)
{
    char* expected = "mov ch, ah\n";
    uint8_t input[] = { 0x88, 0xe5 };
    cr_assert(SUCCESS == dcd_decode_chunk(&g_decoder, input, sizeof(input)));
    cr_assert(1 == g_decoder.instructions_count);
    uint8_t output[16] = { 0x00 };
    dcd_write_all_assembly(g_decoder.instructions, g_decoder.instructions_count, output, sizeof(output));
    cr_assert(strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

Test(decode__I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY__tests, 
     mov3, .init = default_setup)
{
    char* expected = "mov si, bx\n";
    uint8_t input[] = { 0x89, 0xde };
    cr_assert(SUCCESS == dcd_decode_chunk(&g_decoder, input, sizeof(input)));
    cr_assert(1 == g_decoder.instructions_count);
    uint8_t output[16] = { 0x00 };
    dcd_write_all_assembly(g_decoder.instructions, g_decoder.instructions_count, output, sizeof(output));
    cr_assert(strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

Test(decode__I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY__tests,
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
    uint8_t output[256] = { 0x00 };
    dcd_write_all_assembly(g_decoder.instructions, g_decoder.instructions_count, output, sizeof(output));
    cr_assert(strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

Test(decode__I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY__tests, 
     mov4, .init = default_setup)
{
    char* expected = "mov bp, [5]\n";
    uint8_t input[] = { 0x8b, 0x2e, 0x05, 0x00 };
    cr_assert(SUCCESS == dcd_decode_chunk(&g_decoder, input, sizeof(input)));
    cr_assert(1 == g_decoder.instructions_count);
    uint8_t output[16] = { 0x00 };
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
    uint8_t output[16] = { 0x00 };
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
    uint8_t output[16] = { 0x00 };
    dcd_write_all_assembly(g_decoder.instructions, g_decoder.instructions_count, output, sizeof(output));
    cr_assert(strncmp(expected, output, sizeof(output)) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}
