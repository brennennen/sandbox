
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_addi_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

// MARK: I_RV64I_ADDI Tests
Test(emu_rv64_emulate__I_RV64I_ADDI__tests, addi_1, .init = rv64_emu_addi_default_setup)
{
    uint8_t input[] = { 0x93, 0x02, 0x53, 0x00 }; // addi t0, t1, 5
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(5 == g_emulator.harts[0].registers[5]);
}
