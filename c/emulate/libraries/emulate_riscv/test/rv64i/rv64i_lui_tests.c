
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_lui_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

// MARK: I_RV64I_LUI Tests
Test(emu_rv64_emulate__I_RV64I_lui__tests, lui_1, .init = rv64_emu_lui_default_setup)
{
    uint8_t input[] = { 0x12, 0x34, 0x52, 0xb7 }; // lui t0, 0x12345\n
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(1 == g_emulator.instructions_count);
    int64_t res = 0x12345 << 12; // 0x12345 is the upper 20 bits, zero out the lower 12, then sign extend to 64 bits
    cr_assert(res == g_emulator.registers.regs[RV64_REG_T0]);
}
