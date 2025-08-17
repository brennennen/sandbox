
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"
#include "rv64/rv64_disassemble.h"

static emulator_rv64_t g_emulator;

void rv64m_disassemble_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

#define ASSERT_STR_WITH_LOG(expected, actual, max_count) \
    cr_assert(strncmp(expected, actual, max_count) == 0, \
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, actual);

// MARK: mul Tests
Test(emu_rv64_disassemble__mul__tests, mul_1, .init = rv64m_disassemble_default_setup)
{
    char* expected = "mul t0, t1, t2\n";
    uint8_t input[] = { 0xb3, 0x02, 0x73, 0x02 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: mulh Tests
Test(emu_rv64_disassemble__mulh__tests, mulh_1, .init = rv64m_disassemble_default_setup)
{
    char* expected = "mulh t0, t1, t2\n";
    uint8_t input[] = { 0xb3, 0x12, 0x73, 0x02 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: mulhsu Tests
Test(emu_rv64_disassemble__mulhsu__tests, mulhsu_1, .init = rv64m_disassemble_default_setup)
{
    char* expected = "mulhsu t0, t1, t2\n";
    uint8_t input[] = { 0xb3, 0x22, 0x73, 0x02 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: mulhu Tests
Test(emu_rv64_disassemble__mulhu__tests, mulhu_1, .init = rv64m_disassemble_default_setup)
{
    char* expected = "mulhu t0, t1, t2\n";
    uint8_t input[] = { 0xb3, 0x32, 0x73, 0x02 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: div Tests
Test(emu_rv64_disassemble__div__tests, div_1, .init = rv64m_disassemble_default_setup)
{
    char* expected = "div t0, t1, t2\n";
    uint8_t input[] = { 0xb3, 0x42, 0x73, 0x02 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: divu Tests
Test(emu_rv64_disassemble__divu__tests, divu_1, .init = rv64m_disassemble_default_setup)
{
    char* expected = "divu t0, t1, t2\n";
    uint8_t input[] = { 0xb3, 0x52, 0x73, 0x02 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: rem Tests
Test(emu_rv64_disassemble__rem__tests, rem_1, .init = rv64m_disassemble_default_setup)
{
    char* expected = "rem t0, t1, t2\n";
    uint8_t input[] = { 0xb3, 0x62, 0x73, 0x02 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: remu Tests
Test(emu_rv64_disassemble__remu__tests, remu_1, .init = rv64m_disassemble_default_setup)
{
    char* expected = "remu t0, t1, t2\n";
    uint8_t input[] = { 0xb3, 0x72, 0x73, 0x02 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: mulw Tests
Test(emu_rv64_disassemble__mulw__tests, mulw_1, .init = rv64m_disassemble_default_setup)
{
    char* expected = "mulw t0, t1, t2\n";
    uint8_t input[] = { 0xbb, 0x02, 0x73, 0x02 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: divw Tests
Test(emu_rv64_disassemble__divw__tests, divw_1, .init = rv64m_disassemble_default_setup)
{
    char* expected = "divw t0, t1, t2\n";
    uint8_t input[] = { 0xbb, 0x42, 0x73, 0x02 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: divuw Tests
Test(emu_rv64_disassemble__divuw__tests, divuw_1, .init = rv64m_disassemble_default_setup)
{
    char* expected = "divuw t0, t1, t2\n";
    uint8_t input[] = { 0xbb, 0x52, 0x73, 0x02 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: remw Tests
Test(emu_rv64_disassemble__remw__tests, remw_1, .init = rv64m_disassemble_default_setup)
{
    char* expected = "remw t0, t1, t2\n";
    uint8_t input[] = { 0xbb, 0x62, 0x73, 0x02 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: remuw Tests
Test(emu_rv64_disassemble__remuw__tests, remuw_1, .init = rv64m_disassemble_default_setup)
{
    char* expected = "remuw t0, t1, t2\n";
    uint8_t input[] = { 0xbb, 0x72, 0x73, 0x02 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}
