#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/modules/rv64c_compressed.h"
#include "rv64/rv64_decode_instruction.h"
#include "rv64/rv64_disassemble.h"
#include "rv64/rv64_instructions.h"

#define ASSERT_STR_WITH_LOG(expected, actual, max_count)                                           \
    cr_assert(                                                                                     \
        strncmp(expected, actual, max_count) == 0, "expected:\n'%s'\n\nactual:\n'%s'\n", expected, \
        actual                                                                                     \
    );

static rv64_disassembler_t g_disassembler;

void rv64c_disassemble_default_setup(void) {
    memset(&g_disassembler, 0, sizeof(rv64_disassembler_t));
    rv64_disassemble_init(&g_disassembler);
}

// 27.3 load and store
// MARK: c.lw tests
Test(emu_rv64c_disassemble__I_RV64C_CLW__tests, clw_1, .init = rv64c_disassemble_default_setup) {
    char* expected = "lw a0, 4(a1)\n";  // c.lw a0, 4(a1)
    uint8_t input[] = {0xc8, 0x41};
    char output[32] = {'\0'};
    cr_assert(
        SUCCESS
        == rv64_disassemble_chunk(&g_disassembler, input, sizeof(input), output, sizeof(output))
    );
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: c.ld tests
Test(emu_rv64c_disassemble__I_RV64C_CLD__tests, cld_1, .init = rv64c_disassemble_default_setup) {
    char* expected = "ld a0, 8(a1)\n";  // c.ld a0, 8(a1)
    uint8_t input[] = {0x88, 0x65};
    char output[32] = {'\0'};
    cr_assert(
        SUCCESS
        == rv64_disassemble_chunk(&g_disassembler, input, sizeof(input), output, sizeof(output))
    );
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// C.FLW - RV32 only?
// C.FLD
// MARK: c.sw tests
Test(emu_rv64c_disassemble__I_RV64C_CSW__tests, csw_1, .init = rv64c_disassemble_default_setup) {
    char* expected = "sw a0, 4(a1)\n";  // c.sw a0, 4(a1)
    uint8_t input[] = {0xc8, 0xc1};
    char output[32] = {'\0'};
    cr_assert(
        SUCCESS
        == rv64_disassemble_chunk(&g_disassembler, input, sizeof(input), output, sizeof(output))
    );
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: c.sd tests
Test(emu_rv64c_disassemble__I_RV64C_CSD__tests, csd_1, .init = rv64c_disassemble_default_setup) {
    char* expected = "sd a0, 8(a1)\n";  // c.sd a0, 8(a1)
    uint8_t input[] = {0x88, 0xe5};
    char output[32] = {'\0'};
    cr_assert(
        SUCCESS
        == rv64_disassemble_chunk(&g_disassembler, input, sizeof(input), output, sizeof(output))
    );
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// C.FSW - RV32 only?
// C.FSD
// 27.4 control transfer
// C.J
// cr_assert(I_RV64I_JAL == rv64_decode_instruction_tag(rv64c_expand(0xa031))); // j .+12
// C.JALR
// C.JR
// C.JALR
// C.BEQZ
// C.BNEZ
// cr_assert(I_RV64I_BNEZ == rv64_decode_instruction_tag(rv64c_expand(0xa031))); // bnez a0, 20
// 27.5 integer computational instructions
// C.LI
// C.LUI
// ...
// 27.5.1 integer constant-generation instructions
// 27.5.2 integer register-immediate operations
// MARK: c.addi tests
Test(
    emu_rv64c_disassemble__I_RV64C_CADDI__tests,
    caddi_1,
    .init = rv64c_disassemble_default_setup
) {
    char* expected = "addi t1, t1, 16\n";  // c.addi t1, t1, 16
    uint8_t input[] = {0x41, 0x03};
    char output[32] = {'\0'};
    cr_assert(
        SUCCESS
        == rv64_disassemble_chunk(&g_disassembler, input, sizeof(input), output, sizeof(output))
    );
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: c.addiw tests
Test(
    emu_rv64c_disassemble__I_RV64C_CADDIW__tests,
    caddiw_1,
    .init = rv64c_disassemble_default_setup
) {
    char* expected = "addiw t1, t1, 16\n";  // c.addiw t1, t1, 16
    uint8_t input[] = {0x41, 0x23};
    char output[32] = {'\0'};
    cr_assert(
        SUCCESS
        == rv64_disassemble_chunk(&g_disassembler, input, sizeof(input), output, sizeof(output))
    );
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// C.ADDI16SP
// C.ADDI4SPN

// MARK: c.slli tests
Test(
    emu_rv64c_disassemble__I_RV64C_CSLLI__tests,
    cslli_1,
    .init = rv64c_disassemble_default_setup
) {
    char* expected = "slli a0, a0, 1\n";  // c.slli a0, a0, 1
    uint8_t input[] = {0x06, 0x05};
    char output[32] = {'\0'};
    cr_assert(
        SUCCESS
        == rv64_disassemble_chunk(&g_disassembler, input, sizeof(input), output, sizeof(output))
    );
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: c.srli tests
Test(
    emu_rv64c_disassemble__I_RV64C_CSRLI__tests,
    csrli_1,
    .init = rv64c_disassemble_default_setup
) {
    char* expected = "srli a0, a0, 2\n";  // c.srli a0, a0, 2
    uint8_t input[] = {0x09, 0x81};
    char output[32] = {'\0'};
    cr_assert(
        SUCCESS
        == rv64_disassemble_chunk(&g_disassembler, input, sizeof(input), output, sizeof(output))
    );
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: c.srai tests
Test(
    emu_rv64c_disassemble__I_RV64C_CSRAI__tests,
    csrai_1,
    .init = rv64c_disassemble_default_setup
) {
    char* expected = "srai a0, a0, 3\n";  // c.srai a0, a0, 3
    uint8_t input[] = {0x0d, 0x85};
    char output[32] = {'\0'};
    cr_assert(
        SUCCESS
        == rv64_disassemble_chunk(&g_disassembler, input, sizeof(input), output, sizeof(output))
    );
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: c.andi tests
Test(
    emu_rv64c_disassemble__I_RV64C_CANDI__tests,
    candi_1,
    .init = rv64c_disassemble_default_setup
) {
    char* expected = "andi a0, a0, 15\n";  // c.andi a0, a0, 15
    uint8_t input[] = {0x3d, 0x89};
    char output[32] = {'\0'};
    cr_assert(
        SUCCESS
        == rv64_disassemble_chunk(&g_disassembler, input, sizeof(input), output, sizeof(output))
    );
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// 27.5.3 integer register-register operations
// C.MV

// MARK: c.add tests
// Test(emu_rv64c_disassemble__I_RV64C_CADD__tests, cadd_1, .init = rv64c_disassemble_default_setup)
// {
//     char* expected = "add s0, s0, s1\n"; // c.add s0, s0, s1
//     uint8_t input[] = { 0x9a, 0x92 };
//     char output[32] = { '\0' };
//     cr_assert(SUCCESS == rv64_disassemble_chunk(
//         &g_disassembler, input, sizeof(input), output, sizeof(output)));
//     cr_assert(1 == g_disassembler.instructions_count);
//     ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
// }

// MARK: c.and tests
// TODO: 'and fp, fp, s1', 'fp' instead of 's0'
// Test(emu_rv64c_disassemble__I_RV64C_CAND__tests, cand_1, .init = rv64c_disassemble_default_setup)
// {
//     char* expected = "and s0, s0, s1\n"; // c.and s0, s0, s1
//     uint8_t input[] = { 0x65, 0x8c };
//     char output[32] = { '\0' };
//     cr_assert(SUCCESS == rv64_disassemble_chunk(
//         &g_disassembler, input, sizeof(input), output, sizeof(output)));
//     cr_assert(1 == g_disassembler.instructions_count);
//     ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
// }

// MARK: c.or tests
Test(emu_rv64c_disassemble__I_RV64C_COR__tests, cor_1, .init = rv64c_disassemble_default_setup) {
    char* expected = "or a0, a0, a1\n";  // c.or a0, a0, a1
    uint8_t input[] = {0x4d, 0x8d};
    char output[32] = {'\0'};
    cr_assert(
        SUCCESS
        == rv64_disassemble_chunk(&g_disassembler, input, sizeof(input), output, sizeof(output))
    );
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: c.xor tests
Test(emu_rv64c_disassemble__I_RV64C_CXOR__tests, cxor_1, .init = rv64c_disassemble_default_setup) {
    char* expected = "xor a2, a2, a3\n";  // c.xor a2, a2, a3
    uint8_t input[] = {0x35, 0x8e};
    char output[32] = {'\0'};
    cr_assert(
        SUCCESS
        == rv64_disassemble_chunk(&g_disassembler, input, sizeof(input), output, sizeof(output))
    );
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: c.sub tests
Test(emu_rv64c_disassemble__I_RV64C_CSUB__tests, csub_1, .init = rv64c_disassemble_default_setup) {
    char* expected = "sub a4, a4, a5\n";  // c.sub a4, a4, a5
    uint8_t input[] = {0x1d, 0x8f};
    char output[32] = {'\0'};
    cr_assert(
        SUCCESS
        == rv64_disassemble_chunk(&g_disassembler, input, sizeof(input), output, sizeof(output))
    );
    cr_assert(1 == g_disassembler.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}

// MARK: c.addw tests
// TODO: 'addw fp, fp, s1', 'fp' instead of 's0'
// Test(emu_rv64c_disassemble__I_RV64C_CADDW__tests, caddw_1, .init =
// rv64c_disassemble_default_setup)
// {
//     char* expected = "addw s0, s0, s1\n"; // c.addw s0, s0, s1
//     uint8_t input[] = { 0x25, 0x9c };
//     char output[32] = { '\0' };
//     cr_assert(SUCCESS == rv64_disassemble_chunk(
//         &g_disassembler, input, sizeof(input), output, sizeof(output)));
//     cr_assert(1 == g_disassembler.instructions_count);
//     ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
// }

// MARK: c.subw tests
// TODO: 'addw fp, fp, s1', 'fp' instead of 's0'
// Test(emu_rv64c_disassemble__I_RV64C_CSUBW__tests, csubw_1, .init =
// rv64c_disassemble_default_setup)
// {
//     char* expected = "subw s0, s0, s1\n"; // c.subw s0, s0, s1
//     uint8_t input[] = { 0x05, 0x9c };
//     char output[32] = { '\0' };
//     cr_assert(SUCCESS == rv64_disassemble_chunk(
//         &g_disassembler, input, sizeof(input), output, sizeof(output)));
//     cr_assert(1 == g_disassembler.instructions_count);
//     ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
// }

// 27.5.4 defined illegal instruction
// 0
// 27.5.5 nop instruction
// C.NOP
// 27.5.6 breakpoint instruction
// C.EBREAK
// 27.6 Usage of C instructions in LR/SC sequences
// 27.7 HINT instructions
