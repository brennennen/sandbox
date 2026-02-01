#include <criterion/criterion.h>
#include <string.h>
#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_csdsp_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
    g_emulator.harts[0].rv64c_enabled = true;
}

Test(emu_rv64c_emulate__csdsp__tests, csdsp_1, .init = rv64_emu_csdsp_default_setup) {
    g_emulator.harts[0].registers[RV64_REG_SP] = 8;
    g_emulator.harts[0].registers[RV64_REG_S1] = 0x1122334455667788ULL;
    uint8_t input[] = {0x26, 0xe8};  // sd s1, 16(sp) # sdsp
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    uint8_t* mem = g_emulator.harts[0].shared_system->memory;
    printf("c.sdsp: %d\n", mem[24]);
    cr_assert(0x88 == mem[24]);  // sd_imm + sp = 16 + 8 = 24
    cr_assert(0x77 == mem[25]);
    cr_assert(0x66 == mem[26]);
    cr_assert(0x55 == mem[27]);
    cr_assert(0x44 == mem[28]);
    cr_assert(0x33 == mem[29]);
    cr_assert(0x22 == mem[30]);
    cr_assert(0x11 == mem[31]);
}
