/**
 * andi tests
 * `andi rd, rs1, imm12`
 * "logical/bitwise AND" - Both numbers sign extended.
 */

#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_andi_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

Test(emu_rv64_emulate__andi__tests, andi_1, .init = rv64_emu_andi_default_setup)
{
    g_emulator.harts[0].registers[RV64_REG_T0] = 1; // rd
    g_emulator.harts[0].registers[RV64_REG_T1] = 17; // 16 = 0b0001 0001
    uint8_t input[] = { 0x93, 0x72, 0x03, 0x05 }; // `and t0, t1, 80` (80 = 0b0101 0000)
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(16 == g_emulator.harts[0].registers[RV64_REG_T0]); // 80 & 17 = 16 (0b0101 0000 & 0b0001 0001 = 0b0001 0000)
}

// TODO: test with different sized ints, exercise parsing the upper byte of the
// 12 byte immediate, etc.
