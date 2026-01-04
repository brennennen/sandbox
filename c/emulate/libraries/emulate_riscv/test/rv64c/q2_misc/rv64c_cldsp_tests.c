#include <string.h>
#include <criterion/criterion.h>
#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_cldsp_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
    g_emulator.harts[0].rv64c_enabled = true;
}

Test(emu_rv64c_emulate__cldsp__tests, cldsp_1, .init = rv64_emu_cldsp_default_setup)
{
    g_emulator.harts[0].registers[RV64_REG_SP] = 8;
    g_emulator.harts[0].registers[RV64_REG_A0] = 1;
    uint64_t target_address = 24; // SP + lw_imm = 8 + 16 = 24
    g_emulator.harts[0].shared_system->memory[target_address] = 0x42;
    uint8_t input[] = { 0xc2, 0x64 }; // ld s1, 16(sp) # ldsp
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(0x42 == g_emulator.harts[0].registers[RV64_REG_S1]);
}
