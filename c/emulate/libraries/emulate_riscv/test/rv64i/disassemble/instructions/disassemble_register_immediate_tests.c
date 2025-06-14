
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64i/rv64i_emulate.h"

static emulator_rv64i_t g_emulator;

void rv64i_disassemble_register_immediate_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64i_t));
    emu_rv64i_init(&g_emulator);
}

// MARK: I_RV64I_ADDI Tests
Test(emu_rv64i_disassemble__I_RV64I_ADDI__tests, addi_1, .init = rv64i_disassemble_register_immediate_default_setup)
{
    char* expected = "addi t0, t1, 5\n";
    uint8_t input[] = { 0x00, 0x53, 0x02, 0x93 }; // 00530293
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64i_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

// MARK: I_RV64I_SLTI Tests
Test(emu_rv64i_disassemble__I_RV64I_SLTI__tests, slti_1, .init = rv64i_disassemble_register_immediate_default_setup)
{
    char* expected = "slti t3, t2, 0\n";
    uint8_t input[] = { 0x00, 0x03, 0xae, 0x13 }; // 0003ae13
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64i_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}
