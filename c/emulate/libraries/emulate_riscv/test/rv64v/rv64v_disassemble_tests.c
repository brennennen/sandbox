
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"
#include "rv64/rv64_disassemble.h"

static emulator_rv64_t g_emulator;

void rv64v_disassemble_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

#define ASSERT_STR_WITH_LOG(expected, actual, max_count) \
    cr_assert(strncmp(expected, actual, max_count) == 0, \
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, actual);

// MARK: avl and vtype csr configuration tests
Test(emu_rv64v_disassemble__vsetvli__tests, vsetvli_1, .init = rv64v_disassemble_default_setup)
{
    char* expected = "vsetvli a4, a3, e32, m1, ta, ma\n";
    uint8_t input[] = { 0x57, 0xf7, 0x06, 0x0d };
    char output[64] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: load and store
// load unit stride
Test(emu_rv64v_disassemble__vle8_v__tests, vle8_v_1, .init = rv64v_disassemble_default_setup)
{
    char* expected = "vle8.v v0, (a0)\n";
    uint8_t input[] = { 0x07, 0x00, 0x05, 0x02 };
    char output[64] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

Test(emu_rv64v_disassemble__vle16_v__tests, vle16_v_1, .init = rv64v_disassemble_default_setup)
{
    char* expected = "vle16.v v0, (a0)\n";
    uint8_t input[] = { 0x07, 0x50, 0x05, 0x02 };
    char output[64] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

Test(emu_rv64v_disassemble__vle32_v__tests, vle32_v_1, .init = rv64v_disassemble_default_setup)
{
    char* expected = "vle32.v v0, (a0)\n";
    uint8_t input[] = { 0x07, 0x60, 0x05, 0x02 };
    char output[64] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

Test(emu_rv64v_disassemble__vle64_v__tests, vle64_v_1, .init = rv64v_disassemble_default_setup)
{
    char* expected = "vle64.v v0, (a0)\n";
    uint8_t input[] = { 0x07, 0x70, 0x05, 0x02 };
    char output[64] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// store unit stride
Test(emu_rv64v_disassemble__vse8_v__tests, vse8_v_1, .init = rv64v_disassemble_default_setup)
{
    char* expected = "vse8.v v0, (a0)\n";
    uint8_t input[] = { 0x27, 0x00, 0x05, 0x02 };
    char output[64] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

Test(emu_rv64v_disassemble__vse16_v__tests, vse16_v_1, .init = rv64v_disassemble_default_setup)
{
    char* expected = "vse16.v v0, (a0)\n";
    uint8_t input[] = { 0x27, 0x50, 0x05, 0x02 };
    char output[64] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

Test(emu_rv64v_disassemble__vse32_v__tests, vse32_v_1, .init = rv64v_disassemble_default_setup)
{
    char* expected = "vse32.v v0, (a0)\n";
    uint8_t input[] = { 0x27, 0x60, 0x05, 0x02 };
    char output[64] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

Test(emu_rv64v_disassemble__vse64_v__tests, vse64_v_1, .init = rv64v_disassemble_default_setup)
{
    char* expected = "vse64.v v0, (a0)\n";
    uint8_t input[] = { 0x27, 0x70, 0x05, 0x02 };
    char output[64] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// vector integer arithmetic instructions

// MARK: vector arithmetic tests
// opivv - integer vector vector
// 30.11.1 Vector Single-Width Integer Add and Subtract
Test(emu_rv64v_disassemble__vadd_vv__tests, vadd_vv_1, .init = rv64v_disassemble_default_setup)
{
    char* expected = "vadd.vv v3, v1, v2\n";
    uint8_t input[] = { 0xd7, 0x01, 0x11, 0x02 };
    char output[64] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

Test(emu_rv64v_disassemble__vsub_vv__tests, vsub_vv_1, .init = rv64v_disassemble_default_setup)
{
    char* expected = "vsub.vv v3, v1, v2\n";
    uint8_t input[] = { 0xd7, 0x01, 0x11, 0x0a };
    char output[64] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// opivx - integer vector scalar
Test(emu_rv64v_disassemble__vadd_vx__tests, vadd_vx_1, .init = rv64v_disassemble_default_setup)
{
    char* expected = "vadd.vx v3, v1, t2\n";
    uint8_t input[] = { 0xd7, 0xc1, 0x13, 0x02 };
    char output[64] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

Test(emu_rv64v_disassemble__vsub_vx__tests, vsub_vx_1, .init = rv64v_disassemble_default_setup)
{
    char* expected = "vsub.vx v3, v1, t2\n";
    uint8_t input[] = { 0xd7, 0xc1, 0x13, 0x0a };
    char output[64] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// opivi - integer vector immediate
Test(emu_rv64v_disassemble__vadd_vi__tests, vadd_vi_1, .init = rv64v_disassemble_default_setup)
{
    char* expected = "vadd.vi v2, v1, 12\n";
    uint8_t input[] = { 0x57, 0x31, 0x16, 0x02 };
    char output[64] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

Test(emu_rv64v_disassemble__vrsub_vi__tests, vrsub_vi_1, .init = rv64v_disassemble_default_setup)
{
    char* expected = "vrsub.vi v2, v1, 12\n";
    uint8_t input[] = { 0x57, 0x31, 0x16, 0x0e };
    char output[64] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}
