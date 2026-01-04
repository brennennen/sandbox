#include <string.h>
#include <criterion/criterion.h>
#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_clw_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
    g_emulator.harts[0].rv64c_enabled = true;
}

Test(emu_rv64c_emulate__clw__tests, clw_1, .init = rv64_emu_clw_default_setup)
{
    g_emulator.harts[0].shared_system->memory[0x2004] = 0x12;
    g_emulator.harts[0].registers[RV64_REG_A1] = 0x2000;
    uint8_t input[] = { 0xc8, 0x41 }; // c.lw a0, 4(a1)
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(0x12 == g_emulator.harts[0].registers[RV64_REG_A0]);
}
