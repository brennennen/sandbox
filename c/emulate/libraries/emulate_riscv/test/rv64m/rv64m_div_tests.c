#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_div_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

Test(emu_rv64_emulate__div__tests, div_1, .init = rv64_emu_div_default_setup)
{
    g_emulator.harts[0].registers[RV64_REG_T1] = 12;
    g_emulator.harts[0].registers[RV64_REG_T2] = 4;
    uint8_t input[] = { 0xb3, 0x42, 0x73, 0x02 }; // div t0, t1, t2
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    // 12 / 4 = 3
    cr_assert(3 == g_emulator.harts[0].registers[RV64_REG_T0]);
}
