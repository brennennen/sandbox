/**
 * Emulation tests for the "mov" instruction.
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

#include "libraries/emulate8086/include/emulate8086.h"


// Use of this global "g_decoder" is to try and reduce the amount of code per test. It's reset
// after each test and has a large default instructions buffer.
static emulator_t g_decoder;
void emu_add_default_setup(void) {
    memset(&g_decoder, 0, sizeof(emulator_t));
    emu_init(&g_decoder);
}

// MARK: 1. I_ADD
// TODO

// MARK: 2. I_ADD_IMMEDIATE
Test(emu__I_ADD_IMMEDIATE__tests, add1, .init = emu_add_default_setup)
{
    char* expected = "add cx, 5\n";
    uint8_t input[] = { 0x83, 0xc1, 0x05 }; // 0b10000011
    char output[32] = { 0x00 };
    cr_assert(SUCCESS == emu_emulate_chunk(&g_decoder, input, sizeof(input)));
    cr_assert(1 == g_decoder.instructions_count);
    cr_assert(5 == g_decoder.registers.cx,
        "expected:\n'%d'\n\nactual:\n'%d'\n",
        5, g_decoder.registers.cx);
    // TODO: check carry, zero, parity, etc. flags?
}

// MARK: 3. I_ADD_IMMEDIATE_TO_AX
// TODO
