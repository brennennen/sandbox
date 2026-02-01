
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_sh_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

// MARK: sh tests
Test(emu_rv64_emulate__sh__tests, sh_1, .init = rv64_emu_sh_default_setup) {
    g_emulator.harts[0].registers[RV64_REG_T0] = 12;
    g_emulator.harts[0].registers[RV64_REG_T1] = 0x2000;
    uint8_t input[] = {0x23, 0x10, 0x53, 0x00};  // sh t0, 0(t1)
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(g_emulator.harts[0].shared_system->memory[0x2000] == 12);
}
