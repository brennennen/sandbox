#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_amoswap_w_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

Test(emu_rv64_emulate__amoswap_w__tests, amoswap_w_1, .init = rv64_emu_amoswap_w_default_setup)
{
    g_emulator.memory[0x2000] = 5;
    g_emulator.registers[RV64_REG_T1] = 10;
    g_emulator.registers[RV64_REG_T2] = 0x2000;
    uint8_t input[] = { 0xaf, 0xa2, 0x63, 0x08 }; // amoswap.w t0, t1, (t2)
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(10 == g_emulator.memory[0x2000]);
    cr_assert(5 == g_emulator.registers[RV64_REG_T0]);
}
