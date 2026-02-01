
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_and_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

Test(emu_rv64_emulate__and__tests, and_1, .init = rv64_emu_and_default_setup) {
    g_emulator.harts[0].registers[RV64_REG_T1] = 0x0F;  // 0000 1111
    g_emulator.harts[0].registers[RV64_REG_T2] = 0x11;  // 0001 0001
    uint8_t input[] = {0xb3, 0x72, 0x73, 0x00};         // `and t0, t1, t2`
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(1 == g_emulator.harts[0].registers[RV64_REG_T0]);
}
