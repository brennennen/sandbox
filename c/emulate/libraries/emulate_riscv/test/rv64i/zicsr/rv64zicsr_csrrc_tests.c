
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/binary_utilities.h"
#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_csrrc_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

Test(emu_rv64_emulate__csrrc__tests, csrrs_1, .init = rv64_emu_csrrc_default_setup)
{
    g_emulator.registers[RV64_REG_T1] = 0b1001;
    g_emulator.csrs.fflags = 0b1101;
    uint8_t input[] = { 0xf3, 0x32, 0x13, 0x00 }; // csrrc t0, fflags, t1
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(0b1101 == g_emulator.registers[RV64_REG_T0]);
    cr_assert(0b0100 == g_emulator.csrs.fflags);
}
