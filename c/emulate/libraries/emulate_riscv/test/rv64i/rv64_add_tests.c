
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_add_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

// MARK: I_RV64I_ADD Tests
Test(emu_rv64_emulate__I_RV64I_ADD__tests, addi_1, .init = rv64_emu_add_default_setup)
{
    g_emulator.registers.regs[RV64_REG_T1] = 3;
    g_emulator.registers.regs[RV64_REG_T3] = 5; // 3 + 5 = 8
    uint8_t input[] = { 0x00, 0x53, 0x02, 0x93 }; // add t0, t1, t3
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(8 == g_emulator.registers.regs[RV64_REG_T0]);
}
