
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_ld_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

// MARK: ld tests
Test(emu_rv64_emulate__ld__tests, ld_1, .init = rv64_emu_ld_default_setup)
{
    // TODO: write 8 bytes
    g_emulator.harts[0].shared_system->memory[0x2000] = 0x12;
    g_emulator.harts[0].registers[RV64_REG_T0] = 0x2000; // lui t0, 0x2
    uint8_t input[] = { 0x03, 0xb3, 0x02, 0x00 }; // lb t1, (0)t0
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(0x12 == g_emulator.harts[0].registers[RV64_REG_T1]);
}
