/**
 * slti tests
 * `slti rd, rs1, <imm12>`
 * "Set Less Than Immediate" - Set rd to 1 if rs1 is less than the provided immediate when
 * both numbers are treated as signed.
 */
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_slti_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

Test(emu_rv64_emulate__slti__tests, slti_1, .init = rv64_emu_slti_default_setup)
{
    g_emulator.registers.regs[RV64_REG_T0] = 1; // rd
    g_emulator.registers.regs[RV64_REG_T1] = 10; // rs1
    uint8_t input[] = { 0x93, 0x22, 0x03, 0x00 }; // slti t0, t1, 0
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(0 == g_emulator.registers.regs[RV64_REG_T0]); // 10 is not less than 0
}

Test(emu_rv64_emulate__slti__tests, slti_2, .init = rv64_emu_slti_default_setup)
{
    g_emulator.registers.regs[RV64_REG_T0] = 0; // rd
    g_emulator.registers.regs[RV64_REG_T1] = -10; // rs1
    uint8_t input[] = { 0x93, 0x22, 0x03, 0x00 }; // slti t0, t1, 0
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(1 == g_emulator.registers.regs[RV64_REG_T0]); // -10 is less than 0
}

// TODO: test with different sized ints, exercise parsing the upper byte of the
// 12 byte immediate, etc.
