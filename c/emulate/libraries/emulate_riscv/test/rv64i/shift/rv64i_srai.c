
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_srai_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

Test(emu_rv64_emulate__srai__tests, srai_1, .init = rv64_emu_srai_default_setup)
{
    g_emulator.registers.regs[RV64_REG_T1] = 16; // rs1
    uint8_t input[] = { 0x93, 0x52, 0x43, 0x40 }; // srai t0, t1, 4
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(1 == g_emulator.registers.regs[RV64_REG_T0]); // 16 >> 4
}
