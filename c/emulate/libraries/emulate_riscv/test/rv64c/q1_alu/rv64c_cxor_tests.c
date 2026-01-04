#include <string.h>
#include <criterion/criterion.h>
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_registers.h"

static rv64_emulator_t g_emulator;

void rv64c_emu_cxor_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
    g_emulator.harts[0].rv64c_enabled = true;
}

Test(emu_rv64c_emulate__cxor__tests, cxor_1, .init = rv64c_emu_cxor_default_setup)
{
    g_emulator.harts[0].registers[RV64_REG_A2] = 17; // 0001 0001
    g_emulator.harts[0].registers[RV64_REG_A3] = 1; // 0000 0001
    uint8_t input[] = { 0x35, 0x8e }; // xor a2, a2, a3
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(16 == g_emulator.harts[0].registers[RV64_REG_A2]);
}
