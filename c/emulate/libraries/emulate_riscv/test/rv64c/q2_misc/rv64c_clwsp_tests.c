#include <criterion/criterion.h>
#include <string.h>
#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_clwsp_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
    g_emulator.harts[0].rv64c_enabled = true;
}

Test(emu_rv64c_emulate__clwsp__tests, clwsp_1, .init = rv64_emu_clwsp_default_setup) {
    g_emulator.harts[0].registers[RV64_REG_SP] = 8;
    g_emulator.harts[0].registers[RV64_REG_A0] = 1;
    uint64_t target_address = 20;  // SP + lw_imm = 8 + 12 = 20
    g_emulator.harts[0].shared_system->memory[target_address] = 0x99;
    uint8_t input[] = {0x32, 0x45};  // lw a0, 12(sp) # lwsp
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(0x99 == g_emulator.harts[0].registers[RV64_REG_A0]);
}
