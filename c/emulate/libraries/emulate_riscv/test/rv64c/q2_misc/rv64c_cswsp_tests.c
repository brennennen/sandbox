#include <string.h>
#include <criterion/criterion.h>
#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_cswsp_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
    g_emulator.harts[0].rv64c_enabled = true;
}

Test(emu_rv64c_emulate__cswsp__tests, cswsp_1, .init = rv64_emu_cswsp_default_setup)
{
    g_emulator.harts[0].registers[RV64_REG_SP] = 8;
    g_emulator.harts[0].registers[RV64_REG_A0] = 0xDEADBEEF;
    uint8_t input[] = { 0x2a, 0xc6 }; // sw a0, 12(sp) # swsp
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(0xEF == g_emulator.harts[0].shared_system->memory[20]);
    cr_assert(0xBE == g_emulator.harts[0].shared_system->memory[21]);
    cr_assert(0xAD == g_emulator.harts[0].shared_system->memory[22]);
    cr_assert(0xDE == g_emulator.harts[0].shared_system->memory[23]);
}
