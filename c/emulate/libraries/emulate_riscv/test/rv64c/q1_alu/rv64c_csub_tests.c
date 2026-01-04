#include <string.h>
#include <criterion/criterion.h>
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_registers.h"

static rv64_emulator_t g_emulator;

void rv64c_emu_csub_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
    g_emulator.harts[0].rv64c_enabled = true;
}

Test(emu_rv64c_emulate__csub__tests, csub_1, .init = rv64c_emu_csub_default_setup)
{
    g_emulator.harts[0].registers[RV64_REG_A4] = 10;
    g_emulator.harts[0].registers[RV64_REG_A5] = 1;
    uint8_t input[] = { 0x1d, 0x8f }; // sub a4, a4, a5
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(9 == g_emulator.harts[0].registers[RV64_REG_A4]);
}
