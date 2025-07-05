#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_mulh_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

// MARK: I_RV64M_MULH Tests
Test(emu_rv64_emulate__I_RV64M_MULH__tests, addi_1, .init = rv64_emu_mulh_default_setup)
{
    g_emulator.registers.regs[RV64_REG_T1] = 0xFFFFFFFFFFFFFFFF; // max uint64_t
    g_emulator.registers.regs[RV64_REG_T2] = 2;
    uint8_t input[] = { 0x02, 0x73, 0x32, 0xb3 }; // mulhu t0, t1, t2
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(1 == g_emulator.instructions_count);
    // 2 * 0xFFFFFFFFFFFFFFFF = 0x1FFFFFFFFFFFFFFFE (0x0000000000000001FFFFFFFFFFFFFFFE)
    // mulhu returns the high 64 bits, so 0x1
    cr_assert(1 == g_emulator.registers.regs[RV64_REG_T0]);
}
