#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_div_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

Test(emu_rv64_emulate__div__tests, div_1, .init = rv64_emu_div_default_setup)
{
    g_emulator.registers[RV64_REG_T1] = 12;
    g_emulator.registers[RV64_REG_T2] = 4;
    uint8_t input[] = { 0xb3, 0x42, 0x73, 0x02 }; // div t0, t1, t2
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(1 == g_emulator.instructions_count);
    // 12 / 4 = 3
    cr_assert(3 == g_emulator.registers[RV64_REG_T0]);
}
