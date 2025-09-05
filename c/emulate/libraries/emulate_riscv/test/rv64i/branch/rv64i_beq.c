/**
 * beq tests
 * `beq rs1, rs2, <label>`
 * `beq rs1, rs2, . + <jump offset>`
 * "Branch if EQual" - If rs1 and rs2 are equal, add the offset to pc (branch out of
 * the mainline execution flow).
 */

#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_beq_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

Test(emu_rv64_emulate__beq__tests, beq_take_branch, .init = rv64_emu_beq_default_setup)
{
    uint8_t input[] = {
        0x13, 0x03, 0xa0, 0x00, // li t1, 10
        0x93, 0x03, 0xa0, 0x00, // li t2, 10
        0x63, 0x04, 0x73, 0x00, // beq t1, t2, next
        0x13, 0x0e, 0xe0, 0x01, // li t3, 30
                               // next:
        0x93, 0x0e, 0x80, 0x02, // li t4, 40
    };
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(4 == g_emulator.instructions_count);
    cr_assert(10 == g_emulator.registers[RV64_REG_T1]);
    cr_assert(10 == g_emulator.registers[RV64_REG_T2]);
    cr_assert(0 == g_emulator.registers[RV64_REG_T3]); // we jumped over setting t3, so should be 0.
    cr_assert(40 == g_emulator.registers[RV64_REG_T4]);
}

Test(emu_rv64_emulate__beq__tests, beq_dont_take_branch, .init = rv64_emu_beq_default_setup)
{
    uint8_t input[] = {
        0x13, 0x03, 0xa0, 0x00, // li t1, 10
        0x93, 0x03, 0x40, 0x01, // li t2, 20
        0x63, 0x04, 0x73, 0x00, // beq t1, t2, next
        0x13, 0x0e, 0xe0, 0x01, // li t3, 30
                                // next:
        0x93, 0x0e, 0x80, 0x02, // li t4, 40
    };
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(5 == g_emulator.instructions_count);
    cr_assert(10 == g_emulator.registers[RV64_REG_T1]);
    cr_assert(20 == g_emulator.registers[RV64_REG_T2]);
    cr_assert(30 == g_emulator.registers[RV64_REG_T3]); // didn't take branch, so should be set.
    cr_assert(40 == g_emulator.registers[RV64_REG_T4]);
}

Test(emu_rv64_emulate__beq__tests, beqz_take_branch, .init = rv64_emu_beq_default_setup)
{
    uint8_t input[] = {
        0x13, 0x03, 0x00, 0x00, // li t1, 0
        0x63, 0x04, 0x03, 0x00, // beqz t1, next
        0x13, 0x0e, 0xe0, 0x01, // li t3, 30
                                // next:
        0x93, 0x0e, 0x80, 0x02, // li t4, 40
    };
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(3 == g_emulator.instructions_count);
    cr_assert(0 == g_emulator.registers[RV64_REG_T1]);
    cr_assert(0 == g_emulator.registers[RV64_REG_T3]); // we jumped over setting t3, so should be 0.
    cr_assert(40 == g_emulator.registers[RV64_REG_T4]);
}

Test(emu_rv64_emulate__beq__tests, beqz_dont_take_branch, .init = rv64_emu_beq_default_setup)
{
    uint8_t input[] = {
        0x13, 0x03, 0xa0, 0x00, // li t1, 10
        0x63, 0x04, 0x03, 0x00, // beqz t1, next
        0x13, 0x0e, 0xe0, 0x01, // li t3, 30
                               // next:
        0x93, 0x0e, 0x80, 0x02, // li t4, 40
    };
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(4 == g_emulator.instructions_count);
    cr_assert(10 == g_emulator.registers[RV64_REG_T1]);
    cr_assert(30 == g_emulator.registers[RV64_REG_T3]); // did not take branch, so should be set.
    cr_assert(40 == g_emulator.registers[RV64_REG_T4]);
}
