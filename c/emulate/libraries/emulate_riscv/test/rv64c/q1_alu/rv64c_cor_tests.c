#include <criterion/criterion.h>
#include <string.h>
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_registers.h"

static rv64_emulator_t g_emulator;

void rv64c_emu_cor_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
    g_emulator.harts[0].rv64c_enabled = true;
}

Test(emu_rv64c_emulate__cor__tests, cor_1, .init = rv64c_emu_cor_default_setup) {
    g_emulator.harts[0].registers[RV64_REG_A0] = 16;  // 16 = 0001 0000
    g_emulator.harts[0].registers[RV64_REG_A1] = 1;   //   1 = 0000 0001
    uint8_t input[] = {0x4d, 0x8d};                   // or a0, a0, a1
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(17 == g_emulator.harts[0].registers[RV64_REG_A0]);
}
