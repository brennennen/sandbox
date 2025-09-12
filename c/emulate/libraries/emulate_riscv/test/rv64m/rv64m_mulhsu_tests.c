#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_mulhsu_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

Test(emu_rv64_emulate__mulhsu__tests, mulhsu_1, .init = rv64_emu_mulhsu_default_setup)
{
    g_emulator.harts[0].registers[RV64_REG_T1] = 0x8000000000000000;
    g_emulator.harts[0].registers[RV64_REG_T2] = 2;
    uint8_t input[] = { 0xb3, 0x22, 0x73, 0x02 }; // mulhsu t0, t1, t2
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(-1 == g_emulator.harts[0].registers[RV64_REG_T0]);
}
