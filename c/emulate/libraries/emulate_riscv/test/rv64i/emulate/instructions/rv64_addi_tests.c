
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_add_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

// MARK: I_RV64I_ADDI Tests
Test(emu_rv64_emulate__I_RV64I_ADDI__tests, addi_1, .init = rv64_emu_add_default_setup)
{
    char* expected = "addi t0, t1, 5\n";
    uint8_t input[] = { 0x00, 0x53, 0x02, 0x93 }; // 00530293
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(5 == g_emulator.registers.regs[5]);
}
