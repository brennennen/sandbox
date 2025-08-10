
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/binary_utilities.h"
#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_csrrw_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

Test(emu_rv64_emulate__csrrw__tests, csrrw_1, .init = rv64_emu_csrrw_default_setup)
{
    g_emulator.registers.regs[RV64_REG_T0] = 3;
    g_emulator.control_status_registers.float_csrs.fflags = 6;
    uint8_t input[] = { 0xf3, 0x92, 0x12, 0x00 }; // csrrw t0, fflags, t0
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(6 == g_emulator.registers.regs[RV64_REG_T0]);
    cr_assert(3 == g_emulator.control_status_registers.float_csrs.fflags);
}
