
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"
#include "rv64/rv64_disassemble.h"

static emulator_rv64_t g_emulator;

void rv64i_disassemble_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

#define ASSERT_STR_WITH_LOG(expected, actual, max_count) \
    cr_assert(strncmp(expected, actual, max_count) == 0, \
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, actual);

/*
 *
 * MARK: "I" (register - immediate)
 *
 */

// MARK: I_RV64I_ADDI Tests
Test(emu_rv64_disassemble__I_RV64I_ADDI__tests, addi_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "addi t0, t1, 5\n";
    uint8_t input[] = { 0x00, 0x53, 0x02, 0x93 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: I_RV64I_SLTI Tests
Test(emu_rv64_disassemble__I_RV64I_SLTI__tests, slti_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "slti t3, t2, 0\n";
    uint8_t input[] = { 0x00, 0x03, 0xae, 0x13 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: I_RV64I_SLTIU Tests
Test(emu_rv64_disassemble__I_RV64I_SLTIU__tests, sltiu_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "sltui t0, t1, 255\n";
    uint8_t input[] = { 0x0f, 0xf3, 0x32, 0x93 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: I_RV64I_XORI Tests
Test(emu_rv64_disassemble__I_RV64I_XORI__tests, xori_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "xori t0, t1, 16\n";
    uint8_t input[] = { 0x01, 0x03, 0x42, 0x93 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: I_RV64I_ORI Tests
Test(emu_rv64_disassemble__I_RV64I_ORI__tests, ori_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "ori t5, t6, 32\n";
    uint8_t input[] = { 0x02, 0x0f, 0xef, 0x13 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: I_RV64I_ANDI Tests
Test(emu_rv64_disassemble__I_RV64I_ANDI__tests, andi_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "andi t3, t4, 64\n";
    uint8_t input[] = { 0x04, 0x0e, 0xfe, 0x13 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

/*
 *
 * MARK: "R" (register - register)
 *
 */

// MARK: I_RV64I_ADD Tests
Test(emu_rv64_disassemble__I_RV64I_ADD__tests, add_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "add t0, t1, t2\n";
    uint8_t input[] = { 0x00, 0x73, 0x02, 0xb3 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: I_RV64I_SUB Tests
Test(emu_rv64_disassemble__I_RV64I_SUB__tests, sub_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "sub t4, t5, t6\n";
    uint8_t input[] = { 0x41, 0xff, 0x0e, 0xb3 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: I_RV64I_SLL Tests
Test(emu_rv64_disassemble__I_RV64I_SLL__tests, sll_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "sll t6, t0, t1\n";
    uint8_t input[] = { 0x00, 0x62, 0x9f, 0xb3 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: I_RV64I_SLT Tests
Test(emu_rv64_disassemble__I_RV64I_SLT__tests, slt_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "slt t0, t1, t2\n";
    uint8_t input[] = { 0x00, 0x73, 0x22, 0xb3 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: I_RV64I_SLTU Tests
Test(emu_rv64_disassemble__I_RV64I_SLTU__tests, sltu_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "sltu a0, a1, a2\n";
    uint8_t input[] = { 0x00, 0xc5, 0xb5, 0x33 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: I_RV64I_XOR Tests
Test(emu_rv64_disassemble__I_RV64I_XOR__tests, xor_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "xor a0, t2, s1\n";
    uint8_t input[] = { 0x00, 0x93, 0xc5, 0x33 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: I_RV64I_SRL Tests
Test(emu_rv64_disassemble__I_RV64I_SRL__tests, srl_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "srl s1, s2, s3\n";
    uint8_t input[] = { 0x01, 0x39, 0x54, 0xb3 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}
