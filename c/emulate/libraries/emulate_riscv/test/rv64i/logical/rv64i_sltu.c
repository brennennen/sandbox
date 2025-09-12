
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_sltu_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

Test(emu_rv64_emulate__sltu__tests, sltu_1, .init = rv64_emu_sltu_default_setup)
{
    g_emulator.harts[0].registers[RV64_REG_T0] = 0; // rd
    g_emulator.harts[0].registers[RV64_REG_T1] = 10; // rs1
    g_emulator.harts[0].registers[RV64_REG_T2] = 20; // rs2
    uint8_t input[] = { 0xb3, 0x32, 0x73, 0x00 }; // sltu t0, t1, t2
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert(1 == g_emulator.harts[0].registers[RV64_REG_T0]); // 10 is less than 20
}
