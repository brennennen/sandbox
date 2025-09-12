
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"
#include "rv64/rv64_disassemble.h"

static rv64_disassembler_t g_disassembler;

void rv64a_disassemble_default_setup(void) {
    memset(&g_disassembler, 0, sizeof(rv64_disassembler_t));
    rv64_disassemble_init(&g_disassembler);
}

#define ASSERT_STR_WITH_LOG(expected, actual, max_count) \
    cr_assert(strncmp(expected, actual, max_count) == 0, \
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, actual);

// TODO: lr.w, sc.w

// MARK: amoswap.w Tests
Test(emu_rv64_disassemble__amoswap_w__tests, amoswap_w_1, .init = rv64a_disassemble_default_setup)
{
    char* expected = "amoswap.w t0, t1, (t2)\n";
    // 08 63 a2 af
    // 0xaf, 0xa2, 0x63, 0x08
    uint8_t input[] = { 0xaf, 0xa2, 0x63, 0x08 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == rv64_disassemble_chunk(
        &g_disassembler, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: amoadd.w Tests
Test(emu_rv64_disassemble__amoadd_w__tests, amoadd_w_1, .init = rv64a_disassemble_default_setup)
{
    char* expected = "amoadd.w t0, t1, (t2)\n";
    uint8_t input[] = { 0xaf, 0xa2, 0x63, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == rv64_disassemble_chunk(
        &g_disassembler, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: amoxor.w Tests
Test(emu_rv64_disassemble__amoxor_w__tests, amoxor_w_1, .init = rv64a_disassemble_default_setup)
{
    char* expected = "amoxor.w t0, t1, (t2)\n";
    uint8_t input[] = { 0xaf, 0xa2, 0x63, 0x20 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == rv64_disassemble_chunk(
        &g_disassembler, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: amoand.w Tests
Test(emu_rv64_disassemble__amoand_w__tests, amoand_w_1, .init = rv64a_disassemble_default_setup)
{
    char* expected = "amoand.w t0, t1, (t2)\n";
    uint8_t input[] = { 0xaf, 0xa2, 0x63, 0x60 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == rv64_disassemble_chunk(
        &g_disassembler, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: amoor.w Tests
Test(emu_rv64_disassemble__amoor_w__tests, amoor_w_1, .init = rv64a_disassemble_default_setup)
{
    char* expected = "amoor.w t0, t1, (t2)\n";
    uint8_t input[] = { 0xaf, 0xa2, 0x63, 0x40 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == rv64_disassemble_chunk(
        &g_disassembler, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: amomin.w Tests
Test(emu_rv64_disassemble__amomin_w__tests, amomin_w_1, .init = rv64a_disassemble_default_setup)
{
    char* expected = "amomin.w t0, t1, (t2)\n";
    uint8_t input[] = { 0xaf, 0xa2, 0x63, 0x80 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == rv64_disassemble_chunk(
        &g_disassembler, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: amomax.w Tests
Test(emu_rv64_disassemble__amomax_w__tests, amomax_w_1, .init = rv64a_disassemble_default_setup)
{
    char* expected = "amomax.w t0, t1, (t2)\n";
    uint8_t input[] = { 0xaf, 0xa2, 0x63, 0xa0 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == rv64_disassemble_chunk(
        &g_disassembler, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: amominu.w Tests
Test(emu_rv64_disassemble__amominu_w__tests, amominu_w_1, .init = rv64a_disassemble_default_setup)
{
    char* expected = "amominu.w t0, t1, (t2)\n";
    uint8_t input[] = { 0xaf, 0xa2, 0x63, 0xc0 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == rv64_disassemble_chunk(
        &g_disassembler, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: amomaxu.w Tests
Test(emu_rv64_disassemble__amomaxu_w__tests, amomaxu_w_1, .init = rv64a_disassemble_default_setup)
{
    char* expected = "amomaxu.w t0, t1, (t2)\n";
    uint8_t input[] = { 0xaf, 0xa2, 0x63, 0xe0 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == rv64_disassemble_chunk(
        &g_disassembler, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// TODO: lr.d, sc.d

// MARK: amoswap.d Tests
Test(emu_rv64_disassemble__amoswap_d__tests, amoswap_d_1, .init = rv64a_disassemble_default_setup)
{
    char* expected = "amoswap.d t0, t1, (t2)\n";
    uint8_t input[] = { 0xaf, 0xb2, 0x63, 0x08 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == rv64_disassemble_chunk(
        &g_disassembler, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: amoadd.d Tests
Test(emu_rv64_disassemble__amoadd_d__tests, amoadd_d_1, .init = rv64a_disassemble_default_setup)
{
    char* expected = "amoadd.d t0, t1, (t2)\n";
    uint8_t input[] = { 0xaf, 0xb2, 0x63, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == rv64_disassemble_chunk(
        &g_disassembler, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// TODO: amoxor.d - amomaxu.d


