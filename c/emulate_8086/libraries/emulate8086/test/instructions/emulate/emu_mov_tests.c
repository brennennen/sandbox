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

//
// MARK: MOV
//

// Use of this global "g_emulator" is to try and reduce the amount of code per test. It's reset
// after each test and has a large default instructions buffer.
static emulator_t g_emulator;

void emu_mov_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_t));
    emu_init(&g_emulator);
}

// MARK: 1. I_MOVE
Test(emu__I_MOVE__tests, mov1, .init = emu_mov_default_setup)
{
    char* expected = "mov cx, bx\n";
    uint8_t input[] = { 0x89, 0xd9 };
    char output[32] = { 0x00 };
    g_emulator.registers.bx = 5;
    cr_assert(SUCCESS == emu_emulate_chunk(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(2 == g_emulator.registers.ip);
    cr_assert(g_emulator.registers.bx == g_emulator.registers.cx,
        "expected:\n'%d'\n\nactual:\n'%d'\n",
        g_emulator.registers.bx, g_emulator.registers.cx);
}

// MARK: 2. I_MOVE_IMMEDIATE
// TODO

// MARK: 3. I_MOVE_IMMEDIATE_TO_REGISTER
Test(emu__I_MOVE_IMMEDIATE_TO_REGISTER__tests, mov1, .init = emu_mov_default_setup)
{
    char* expected = "mov cx, 10\n";
    uint8_t input[] = { 0xb9, 0x0a, 0x00 }; // 0b10111001 0b00001010 0b00000000
    char output[32] = { 0x00 };
    cr_assert(SUCCESS == emu_emulate_chunk(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(3 == g_emulator.registers.ip);
    cr_assert(10 == g_emulator.registers.cx,
        "expected:\n'%d'\n\nactual:\n'%d'\n",
        10, g_emulator.registers.cx);
}

Test(emu__I_MOVE_IMMEDIATE_TO_REGISTER__tests, mov2, .init = emu_mov_default_setup)
{
    char* expected = "mov cx, 502\n";
    uint8_t input[] = { 0xb9, 0xf6, 0x01 }; // 0b10111001 0b11110110 0b00000001
    char output[32] = { 0x00 };
    cr_assert(SUCCESS == emu_emulate_chunk(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(3 == g_emulator.registers.ip);
    cr_assert(502 == g_emulator.registers.cx,
        "expected:\n'%d'\n\nactual:\n'%d'\n",
        502, g_emulator.registers.cx);
}

// MARK: 4. I_MOVE_TO_AX
// TODO

// MARK: 5. I_MOVE_AX
// TODO

// MARK: 6. I_MOVE_TO_SEGMENT_REGISTER
// TODO

// MARK: 7. I_MOVE_SEGMENT_REGISTER
// TODO
