/**
 * bge tests
 * `bge rs1, rs2, <label>`
 * `bge rs1, rs2, . + <jump offset>`
 * "Branch Greater Equal than" - If rs1 is greater or equal than, add the offset to pc
 * (branch out of the mainline execution flow).
 */

#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_bge_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

Test(emu_rv64_emulate__bge__tests, bge_take_branch, .init = rv64_emu_bge_default_setup)
{
    uint8_t input[] = {
        0x01, 0x40, 0x03, 0x13, // li t1, 20
        0x00, 0xa0, 0x03, 0x93, // li t2, 10
        0x00, 0x73, 0x54, 0x63, // bge t1, t2, next
        0x01, 0xe0, 0x0e, 0x13, // li t3, 30
                                // next:
        0x02, 0x80, 0x0e, 0x93, // li t4, 40
    };
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(4 == g_emulator.instructions_count);
    cr_assert(20 == g_emulator.registers.regs[RV64_REG_T1]);
    cr_assert(10 == g_emulator.registers.regs[RV64_REG_T2]);
    cr_assert(0 == g_emulator.registers.regs[RV64_REG_T3]); // we jumped over setting t3, so should be 0.
    cr_assert(40 == g_emulator.registers.regs[RV64_REG_T4]);
}