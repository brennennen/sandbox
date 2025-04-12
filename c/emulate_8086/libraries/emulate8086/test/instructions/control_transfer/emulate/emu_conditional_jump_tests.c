/**
 * Emulate tests for conditional jump instructions.
 */
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"
#include "libraries/emulate8086/include/emu_registers.h"
#include "libraries/emulate8086/include/emulate8086.h"

//
// MARK: JNZ/JNE
//

static emulator_t g_emulator;

void emulate_conditional_jump_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_t));
    emu_init(&g_emulator);
}

Test(emulate__conditional_jump__tests, jnz, .init = emulate_conditional_jump_default_setup)
{
// loop_start:
// jnz loop_start
    emu_reg_set_flag(&g_emulator.registers.flags, FLAG_ZF_MASK);
    uint8_t input[] = { 0x75, 0xfe }; // "jne $+0"
    cr_assert(SUCCESS == emu_emulate_chunk(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(2 == g_emulator.registers.ip);
}

Test(emulate__conditional_jump__tests, jnz2, .init = emulate_conditional_jump_default_setup)
{
// mov cx, 3
// mov bx, 1000
// loop_start:
// add bx, 10
// sub cx, 1
// jnz loop_start
    uint8_t input[] = {
        0xb9, 0x03, 0x00, 0xbb, 0xe8, 0x03, 0x83, 0xc3, 0x0a, 0x83, 0xe9, 0x01,
        0x75, 0xf8
    };
    cr_assert(SUCCESS == emu_emulate_chunk(&g_emulator, input, sizeof(input)));
    cr_assert(1030 == g_emulator.registers.bx);
}

Test(emulate__conditional_jump__tests, jnz3, .init = emulate_conditional_jump_default_setup)
{
// mov dx, 6
// mov si, 0
// init_loop_start:
//     add si, 2
//     cmp si, dx
//     jnz init_loop_start
    uint8_t input[] = {
        0xba, 0x06, 0x00, 0xbe, 0x00, 0x00, 0x83, 0xc6, 0x02, 0x39, 0xd6, 0x75,
        0xf9
    };
    cr_assert(SUCCESS == emu_emulate_chunk(&g_emulator, input, sizeof(input)));
    cr_assert(6 == g_emulator.registers.si);
}

Test(emulate__conditional_jump__tests, jnz4, .init = emulate_conditional_jump_default_setup)
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
    uint8_t input[] = {
        0xba, 0x06, 0x00, 0xbd, 0xe8, 0x03, 0xbe, 0x00, 0x00, 0x89, 0x32, 0x83,
        0xc6, 0x02, 0x39, 0xd6, 0x75, 0xf7
    };
    cr_assert(SUCCESS == emu_emulate_chunk(&g_emulator, input, sizeof(input)));
    printf("si: %d\n", g_emulator.registers.si);
    cr_assert(6 == g_emulator.registers.si);
}

Test(emulate__conditional_jump__tests, jnz5, .init = emulate_conditional_jump_default_setup)
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
    uint8_t input[] = {
        0xba, 0x06, 0x00, 0xbd, 0xe8, 0x03, 0xbe, 0x00, 0x00, 0x89, 0x32, 0x83,
        0xc6, 0x02, 0x39, 0xd6, 0x75, 0xf7, 0xbb, 0x00, 0x00, 0xbe, 0x00, 0x00,
        0x8b, 0x0a, 0x01, 0xcb, 0x83, 0xc6, 0x02, 0x39, 0xd6, 0x75, 0xf5
    };
    cr_assert(SUCCESS == emu_emulate_chunk(&g_emulator, input, sizeof(input)));
    cr_assert(6 == g_emulator.registers.bx);
    cr_assert(4 == g_emulator.registers.cx);
    cr_assert(6 == g_emulator.registers.dx);
    cr_assert(6 == g_emulator.registers.si);
    cr_assert(35 == g_emulator.registers.ip);
    cr_assert(FLAG_ZF_MASK == g_emulator.registers.flags);
}
