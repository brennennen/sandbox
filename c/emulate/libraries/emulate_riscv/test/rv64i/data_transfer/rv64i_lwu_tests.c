/**
 * lwu tests
 * `lwu rd, <offset>(rs1)`
 * "Load Word Unsigned" - Loads a 4 byte (32 bit) word from memory as an unsigned value
 * (zero-extends to 64 bits) and stores it in rd.
 */

#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_lwu_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

// MARK: lw tests
Test(emu_rv64_emulate__lwu__tests, lwu_1, .init = rv64_emu_lwu_default_setup) {
    g_emulator.harts[0].shared_system->memory[0x2000] = 0x12;
    g_emulator.harts[0].registers[RV64_REG_T0] = 0x2000;
    uint8_t input[] = {0x03, 0xe3, 0x02, 0x00};  // lwu t1, 0(t0)
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(0x12 == g_emulator.harts[0].registers[RV64_REG_T1]);
}

// TODO: test with numbers that 0 extend from 32 to 64 bit
