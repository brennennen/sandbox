
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_auipc_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

// MARK: auipc Tests
Test(emu_rv64_emulate__auipc__tests, auipc_1, .init = rv64_emu_auipc_default_setup)
{
    uint8_t input[] = { 0x97, 0x52, 0x34, 0x12 }; // auipc t0, 0x12345
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    int64_t res = (0x12345 << 12) + PROGRAM_START; // 0x12345 is the upper 20 bits, zero out the lower 12, then sign extend to 64 bits
    cr_assert(res == g_emulator.harts[0].registers[RV64_REG_T0]);
    cr_assert(PROGRAM_START + 4 == g_emulator.harts[0].pc);
}
