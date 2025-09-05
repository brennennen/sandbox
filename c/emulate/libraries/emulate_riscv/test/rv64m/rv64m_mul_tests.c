
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_mul_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

Test(emu_rv64_emulate__mul__tests, mul_simple, .init = rv64_emu_mul_default_setup)
{
    g_emulator.registers[RV64_REG_T1] = 3;
    g_emulator.registers[RV64_REG_T2] = 5; // 3 * 5 = 15
    uint8_t input[] = { 0xb3, 0x02, 0x73, 0x02 }; // mul t0, t1, t2
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(15 == g_emulator.registers[RV64_REG_T0]);
}

Test(emu_rv64_emulate__mul__tests, mul_overflow, .init = rv64_emu_mul_default_setup)
{
    g_emulator.registers[RV64_REG_T1] = 0xFFFFFFFFFFFFFFFF; // max uint64_t
    g_emulator.registers[RV64_REG_T2] = 2;
    uint8_t input[] = { 0xb3, 0x02, 0x73, 0x02 }; // mul t0, t1, t2
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(1 == g_emulator.instructions_count);
    // 2 * 0xFFFFFFFFFFFFFFFF = 0x1FFFFFFFFFFFFFFFE (0x0000000000000001FFFFFFFFFFFFFFFE)
    // mul returns the low 64 bits, so 0xFFFFFFFFFFFFFFFE (see mulh, mulhu, and mulhsu for high 64 bits)
    cr_assert(0xFFFFFFFFFFFFFFFE == g_emulator.registers[RV64_REG_T0]);
}
