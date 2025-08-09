
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
 * MARK: "U" (upper immediate)
 *
 */
Test(emu_rv64_disassemble__I_RV64I_LUI__tests, lui_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "lui t0, 74565\n"; // lui t0, 0x12345
    uint8_t input[] = { 0xb7, 0x52, 0x34, 0x12 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

Test(emu_rv64_disassemble__I_RV64I_AUIPC__tests, auipc_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "auipc t0, 74565\n"; // auipc t0, 0x12345
    uint8_t input[] = { 0x97, 0x52, 0x34, 0x12 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

/**
 * MARK: "J" (jump type?)
 */

// MARK: JAL tests
Test(emu_rv64_disassemble__I_RV64I_JAL__tests, jal_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "jal ra, . + 12\n"; // "jal ra, add_numbers" (where add_numbers is a symbol/tag a couple instructions (12 bytes) ahead)
    uint8_t input[] = { 0xef, 0x00, 0xc0, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: jalr tests
Test(emu_rv64_disassemble__I_RV64I_JALR__tests, jalr_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "jalr ra, t0, 0\n"; // jalr ra, t0, 0
    uint8_t input[] = { 0xe7, 0x80, 0x02, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

/**
 * MARK: "B" (branch)
 */

// MARK: beq tests
Test(emu_rv64_disassemble__beq__tests, beq_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "beq a1, zero, . + -8\n"; // beq a1, x0, loop_start
    uint8_t input[] = { 0xe3, 0x8c, 0x05, 0xfe };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: bne tests
Test(emu_rv64_disassemble__bne__tests, beq_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "bne a1, zero, . + -12\n"; // bne a1, x0, loop_start
    uint8_t input[] = { 0xe3, 0x9a, 0x05, 0xfe };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: blt tests
Test(emu_rv64_disassemble__blt__tests, beq_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "blt a1, zero, . + -16\n"; // blt a1, x0, loop_start
    uint8_t input[] = { 0xe3, 0xc8, 0x05, 0xfe };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: bge tests
Test(emu_rv64_disassemble__bge__tests, beq_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "bge a1, zero, . + -24\n"; // bge a1, x0, loop_start
    uint8_t input[] = { 0xe3, 0xd4, 0x05, 0xfe };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: bltu tests
Test(emu_rv64_disassemble__bltu__tests, bltu_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "bltu a1, zero, . + -20\n"; // bltu a1, x0, loop_start
    uint8_t input[] = { 0xe3, 0xe6, 0x05 ,0xfe };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: bgeu tests
Test(emu_rv64_disassemble__bgeu__tests, bltu_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "bgeu a1, zero, . + -28\n"; // bgeu a1, x0, loop_start
    uint8_t input[] = { 0xe3, 0xf2, 0x05, 0xfe };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: lb tests
Test(emu_rv64_disassemble__lb__tests, lb_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "lb t1, 0(t0)\n";
    uint8_t input[] = { 0x03, 0x83, 0x02, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: lh tests
Test(emu_rv64_disassemble__lh__tests, lh_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "lh t1, 0(t0)\n";
    uint8_t input[] = { 0x03, 0x93, 0x02, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: lw tests
Test(emu_rv64_disassemble__lw__tests, lw_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "lw t1, 0(t0)\n";
    uint8_t input[] = { 0x03, 0xa3, 0x02, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: lbu tests
Test(emu_rv64_disassemble__lbu__tests, lbu_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "lbu t1, 0(t0)\n";
    uint8_t input[] = { 0x03, 0xc3, 0x02, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: lhu tests
Test(emu_rv64_disassemble__lhu__tests, lhu_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "lhu t1, 0(t0)\n";
    uint8_t input[] = { 0x03, 0xd3, 0x02, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: sb tests
Test(emu_rv64_disassemble__sb__tests, sb_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "sb t0, 0(t1)\n";
    uint8_t input[] = { 0x23, 0x00, 0x53, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: sh tests
Test(emu_rv64_disassemble__sh__tests, sh_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "sh t0, 0(t1)\n";
    uint8_t input[] = { 0x23, 0x10, 0x53, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: sw tests
Test(emu_rv64_disassemble__sw__tests, sw_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "sw t0, 0(t1)\n";
    uint8_t input[] = { 0x23, 0x20, 0x53, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

/*
 *
 * MARK: "I" (register - immediate)
 *
 */

// MARK: addi tests
Test(emu_rv64_disassemble__addi__tests, addi_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "addi t0, t1, 5\n";
    uint8_t input[] = { 0x93, 0x02, 0x53, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: slti Tests
Test(emu_rv64_disassemble__slti__tests, slti_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "slti t3, t2, 0\n";
    uint8_t input[] = { 0x13, 0xae, 0x03, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: sltui Tests
Test(emu_rv64_disassemble__sltui__tests, sltiu_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "sltui t0, t1, 255\n";
    uint8_t input[] = { 0x93, 0x32, 0xf3, 0x0f };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: xori Tests
Test(emu_rv64_disassemble__xori__tests, xori_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "xori t0, t1, 16\n";
    uint8_t input[] = { 0x93, 0x42, 0x03, 0x01 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: ori Tests
Test(emu_rv64_disassemble__ori__tests, ori_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "ori t5, t6, 32\n";
    uint8_t input[] = { 0x13, 0xef, 0x0f, 0x02 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: andi Tests
Test(emu_rv64_disassemble__andi__tests, andi_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "andi t3, t4, 64\n";
    uint8_t input[] = { 0x13, 0xfe, 0x0e, 0x04 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: ssli Tests
Test(emu_rv64_disassemble__slli__tests, slli_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "slli t0, t1, 7\n";
    uint8_t input[] = { 0x93, 0x12, 0x73, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: srli Tests
Test(emu_rv64_disassemble__srli__tests, srli_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "srli t0, t1, 7\n";
    uint8_t input[] = { 0x93, 0x52, 0x73, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: srai Tests
Test(emu_rv64_disassemble__srai__tests, srai_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "srai t0, t1, 7\n";
    uint8_t input[] = { 0x93, 0x52, 0x73, 0x40 };
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

// MARK: add Tests
Test(emu_rv64_disassemble__add__tests, add_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "add t0, t1, t2\n";
    uint8_t input[] = { 0xb3, 0x02, 0x73, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: sub Tests
Test(emu_rv64_disassemble__sub__tests, sub_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "sub t4, t5, t6\n";
    uint8_t input[] = { 0xb3, 0x0e, 0xff, 0x41 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: sll Tests
Test(emu_rv64_disassemble__sll__tests, sll_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "sll t6, t0, t1\n";
    uint8_t input[] = { 0xb3, 0x9f, 0x62, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: slt Tests
Test(emu_rv64_disassemble__slt__tests, slt_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "slt t0, t1, t2\n";
    uint8_t input[] = { 0xb3, 0x22, 0x73, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: sltu Tests
Test(emu_rv64_disassemble__sltu__tests, sltu_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "sltu a0, a1, a2\n";
    uint8_t input[] = { 0x33, 0xb5, 0xc5, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: xor Tests
Test(emu_rv64_disassemble__xor__tests, xor_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "xor a0, t2, s1\n";
    uint8_t input[] = { 0x33, 0xc5, 0x93, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: srl Tests
Test(emu_rv64_disassemble__srl__tests, srl_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "srl s1, s2, s3\n";
    uint8_t input[] = { 0xb3, 0x54, 0x39, 0x01 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: sra Tests
Test(emu_rv64_disassemble__sra__tests, sra_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "sra t0, t1, t2\n";
    uint8_t input[] = { 0xb3, 0x52, 0x73, 0x40 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: or Tests
Test(emu_rv64_disassemble__or__tests, sra_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "or t0, t1, t2\n";
    uint8_t input[] = { 0xb3, 0x62, 0x73, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: and Tests
Test(emu_rv64_disassemble__and__tests, sra_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "and t0, t1, t2\n";
    uint8_t input[] = { 0xb3, 0x72, 0x73, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: fence Tests
Test(emu_rv64_disassemble__fence__tests, fence_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "fence\n";
    uint8_t input[] = { 0x0f, 0x00, 0xf0, 0x0f };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: fence.tso Tests
Test(emu_rv64_disassemble__fence_tso__tests, fence_tso_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "fence.tso\n";
    uint8_t input[] = { 0x0f, 0x00, 0x30, 0x83 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: pause Tests
Test(emu_rv64_disassemble__pause__tests, pause_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "pause\n";
    uint8_t input[] = { 0x0f, 0x00, 0x00, 0x01 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: ecall Tests
Test(emu_rv64_disassemble__ecall__tests, ecall_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "ecall\n";
    uint8_t input[] = { 0x73, 0x00, 0x00, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: ebreak Tests
Test(emu_rv64_disassemble__ebreak__tests, ebreak_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "ebreak\n";
    uint8_t input[] = { 0x73, 0x00, 0x10, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: lwu Tests
Test(emu_rv64_disassemble__lwu__tests, lwu_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "lwu t1, 0(t0)\n";
    uint8_t input[] = { 0x03, 0xe3, 0x02, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: ld Tests
Test(emu_rv64_disassemble__ld__tests, ld_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "ld t1, 0(t0)\n";
    uint8_t input[] = { 0x03, 0xb3, 0x02, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: sd Tests
Test(emu_rv64_disassemble__sd__tests, sd_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "sd t0, 0(t1)\n";
    uint8_t input[] = { 0x23, 0x30, 0x53, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: addiw Tests
Test(emu_rv64_disassemble__addiw__tests, addiw_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "addiw t0, t1, 7\n";
    uint8_t input[] = { 0x9b, 0x02, 0x73, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: slliw Tests
Test(emu_rv64_disassemble__slliw__tests, slliw_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "slliw t0, t1, 7\n";
    uint8_t input[] = { 0x9b, 0x12, 0x73, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: srliw Tests
Test(emu_rv64_disassemble__slrliw__tests, srliw_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "srliw t0, t1, 7\n";
    uint8_t input[] = { 0x9b, 0x52, 0x73, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: sraiw Tests
Test(emu_rv64_disassemble__sraiw__tests, sraiw_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "sraiw t0, t1, 7\n";
    uint8_t input[] = { 0x9b, 0x52, 0x73, 0x40 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: addw Tests
Test(emu_rv64_disassemble__addw__tests, addw_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "addw t0, t1, t2\n";
    uint8_t input[] = { 0xbb, 0x02, 0x73, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: subw Tests
Test(emu_rv64_disassemble__subw__tests, subw_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "subw t0, t1, t2\n";
    uint8_t input[] = { 0xbb, 0x02, 0x73, 0x40 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: sllw Tests
Test(emu_rv64_disassemble__sllw__tests, sllw_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "sllw t0, t1, t2\n";
    uint8_t input[] = { 0xbb, 0x12, 0x73, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: srlw Tests
Test(emu_rv64_disassemble__srlw__tests, srlw_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "srlw t0, t1, t2\n";
    uint8_t input[] = { 0xbb, 0x52, 0x73, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: sraw Tests
Test(emu_rv64_disassemble__sraw__tests, sraw_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "sraw t0, t1, t2\n";
    uint8_t input[] = { 0xbb, 0x52, 0x73, 0x40 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

/*



MARK: Extensions



*/

// MARK: RV64Zifenceei
// MARK: fence.i Tests
Test(emu_rv64_disassemble__fence_i__tests, fence_i_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "fence.i\n";
    uint8_t input[] = { 0x0f, 0x10, 0x00, 0x00 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: RV64Zicsr
// MARK: csrrw Tests
Test(emu_rv64_disassemble__csrrw__tests, csrrw_1, .init = rv64i_disassemble_default_setup)
{
    char* expected = "csrrw t0, mscratch, t0\n";
    uint8_t input[] = { 0xf3, 0x92, 0x02, 0x34 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}


// TODO: other i extensions ???
