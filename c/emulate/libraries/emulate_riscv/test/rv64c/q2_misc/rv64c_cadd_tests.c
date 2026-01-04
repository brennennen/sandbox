#include <string.h>
#include <criterion/criterion.h>
#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_cadd_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
    g_emulator.harts[0].rv64c_enabled = true;
}

Test(emu_rv64c_emulate__cadd__tests, cadd_1, .init = rv64_emu_cadd_default_setup)
{
    g_emulator.harts[0].registers[RV64_REG_T0] = 3;
    g_emulator.harts[0].registers[RV64_REG_T1] = 5;
    uint8_t input[] = { 0x9a, 0x92 }; // c.add t0, t0, t1
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(8 == g_emulator.harts[0].registers[RV64_REG_T0]);
}
