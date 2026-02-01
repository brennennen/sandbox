
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_lb_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

// MARK: lb tests
Test(emu_rv64_emulate__lb__tests, lb_1, .init = rv64_emu_lb_default_setup) {
    g_emulator.harts[0].shared_system->memory[0x2000] = 0x12;
    g_emulator.harts[0].registers[RV64_REG_T0] = 0x2000;  // lui t0, 0x2
    uint8_t input[] = {0x03, 0x83, 0x02, 0x00};           // lb t1, (0)t0
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    printf("%s:t1: %ld\n", __func__, g_emulator.harts[0].registers[RV64_REG_T1]);
    cr_assert(0x12 == g_emulator.harts[0].registers[RV64_REG_T1]);
}
