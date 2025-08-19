#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_remuw_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

Test(emu_rv64_emulate__remuw__tests, remuw_1, .init = rv64_emu_remuw_default_setup)
{
    g_emulator.registers.regs[RV64_REG_T1] = 12;
    g_emulator.registers.regs[RV64_REG_T2] = 5;
    uint8_t input[] = { 0xbb, 0x72, 0x73, 0x02 }; // remuw t0, t1, t2
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(1 == g_emulator.instructions_count);
    // 12 % 5 = 2
    cr_assert(2 == g_emulator.registers.regs[RV64_REG_T0]);
}
