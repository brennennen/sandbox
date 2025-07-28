/**
 * ori tests
 * `ori rd, rs1, imm12`
 * "logical/bitwise OR" - Both numbers sign extended.
 */

#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_ori_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

Test(emu_rv64_emulate__ori__tests, ori_1, .init = rv64_emu_ori_default_setup)
{
    g_emulator.registers.regs[RV64_REG_T0] = 1; // rd
    g_emulator.registers.regs[RV64_REG_T1] = 16; // 16 = 0b0001 0000
    uint8_t input[] = { 0x02, 0x03, 0x62, 0x93 }; // `ori t0, t1, 32` (32 = 0b0010 0000)
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(48 == g_emulator.registers.regs[RV64_REG_T0]); // 16 | 32 = 48 (0b0010 0000 ^ 0b0001 0000 = 0b0011 0000)
}

// TODO: test with different sized ints, exercise parsing the upper byte of the
// 12 byte immediate, etc.
