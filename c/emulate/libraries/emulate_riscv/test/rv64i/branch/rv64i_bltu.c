/**
 * bltu tests
 * `bltu rs1, rs2, <label>`
 * `bltu rs1, rs2, . + <jump offset>`
 * "Branch Less Than" - If rs1 is less than rs2 (unsigned), add the offset to pc (branch out of
 * the mainline execution flow).
 */

#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_bltu_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

Test(emu_rv64_emulate__bltu__tests, bltu_take_branch, .init = rv64_emu_bltu_default_setup)
{
    uint8_t input[] = {
        0x13, 0x03, 0xa0, 0x00, // li t1, 10
        0x93, 0x03, 0x40, 0x01, // li t2, 20
        0x63, 0x64, 0x73, 0x00, // bltu t1, t2, next
        0x13, 0x0e, 0xe0, 0x01, // li t3, 30
                                // next:
        0x93, 0x0e, 0x80, 0x02, // li t4, 40
    };
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(4 == g_emulator.instructions_count);
    cr_assert(10 == g_emulator.registers[RV64_REG_T1]);
    cr_assert(20 == g_emulator.registers[RV64_REG_T2]);
    cr_assert(0 == g_emulator.registers[RV64_REG_T3]); // we jumped over setting t3, so should be 0.
    cr_assert(40 == g_emulator.registers[RV64_REG_T4]);
}