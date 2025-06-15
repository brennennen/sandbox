
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_disassemble_register_immediate_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

// MARK: I_RV64I_ADDI Tests
Test(emu_rv64_disassemble__I_RV64I_ADDI__tests, addi_1, .init = rv64_disassemble_register_immediate_default_setup)
{
    char* expected = "addi t0, t1, 5\n";
    uint8_t input[] = { 0x00, 0x53, 0x02, 0x93 }; // 00530293
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

// MARK: I_RV64I_SLTI Tests
Test(emu_rv64_disassemble__I_RV64I_SLTI__tests, slti_1, .init = rv64_disassemble_register_immediate_default_setup)
{
    char* expected = "slti t3, t2, 0\n";
    uint8_t input[] = { 0x00, 0x03, 0xae, 0x13 }; // 0003ae13
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

// MARK: I_RV64I_SLTIU Tests
Test(emu_rv64_disassemble__I_RV64I_SLTIU__tests, sltiu_1, .init = rv64_disassemble_register_immediate_default_setup)
{
    char* expected = "sltui t0, t1, 255\n";
    uint8_t input[] = { 0x0f, 0xf3, 0x32, 0x93 }; // 0ff33293
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

// MARK: I_RV64I_XORI Tests
Test(emu_rv64_disassemble__I_RV64I_XORI__tests, xori_1, .init = rv64_disassemble_register_immediate_default_setup)
{
    char* expected = "xori t0, t1, 16\n";
    uint8_t input[] = { 0x01, 0x03, 0x42, 0x93 }; // 01034293
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

// MARK: I_RV64I_ORI Tests
Test(emu_rv64_disassemble__I_RV64I_ORI__tests, ori_1, .init = rv64_disassemble_register_immediate_default_setup)
{
    char* expected = "ori t5, t6, 32\n";
    uint8_t input[] = { 0x02, 0x0f, 0xef, 0x13 }; // 020fef13
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}

// MARK: I_RV64I_ANDI Tests
Test(emu_rv64_disassemble__I_RV64I_ANDI__tests, andi_1, .init = rv64_disassemble_register_immediate_default_setup)
{
    char* expected = "andi t3, t4, 64\n";
    uint8_t input[] = { 0x04, 0x0e, 0xfe, 0x13 }; // 040efe13
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}
