#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_instructions.h"
#include "rv64/rv64_decode_instruction.h"
#include "rv64/instructions/rv64c_compressed.h"

// MARK: Instructions
Test(emu_rv64c_expand_tag_tests, expand_instruction_tests) {
    // 27.3 load and store
    cr_assert(I_RV64I_LW == rv64_decode_instruction_tag(rv64c_expand(0x41c8))); // lw a0, 4(a1)
    cr_assert(I_RV64I_LD == rv64_decode_instruction_tag(rv64c_expand(0x6588))); // ld a0, 8(a1)
    // C.FLW - RV32 only?
    //cr_assert(I_RV64D_FLD == rv64_decode_instruction_tag(rv64c_expand(0x2588))); // fld fa0, 8(a1)
    cr_assert(I_RV64I_SW == rv64_decode_instruction_tag(rv64c_expand(0xc1c8))); // sw a0, 4(a1)
    cr_assert(I_RV64I_SD == rv64_decode_instruction_tag(rv64c_expand(0xe588))); // sd a0, 8(a1)
    // C.FSW - RV32 only?
    //cr_assert(I_RV64D_FSD == rv64_decode_instruction_tag(rv64c_expand(0xa588))); // fsd fa0, 8(a1)
    // 27.4 control transfer
    // C.J
    //cr_assert(I_RV64I_JAL == rv64_decode_instruction_tag(rv64c_expand(0xa031))); // j .+12
    // C.JALR
    // C.JR
    // C.JALR
    // C.BEQZ
    // C.BNEZ
    //cr_assert(I_RV64I_BNEZ == rv64_decode_instruction_tag(rv64c_expand(0xa031))); // bnez a0, 20 
    // 27.5 integer computational instructions
    // C.LI
    // C.LUI
    // 27.5.1 integer constant-generation instructions
    // 27.5.2 integer register-immediate operations
    cr_assert(I_RV64I_ADDI == rv64_decode_instruction_tag(rv64c_expand(0x0341))); // addi t1, t1, 16
    cr_assert(I_RV64I_ADDIW == rv64_decode_instruction_tag(rv64c_expand(0x2341))); // addiw t1, t1, 16
    //cr_assert(I_RV64I_ADDI == rv64_decode_instruction_tag(rv64c_expand(0x1101))); // addi sp, sp, -32 # addi16sp
    //cr_assert(I_RV64I_ADDI == rv64_decode_instruction_tag(rv64c_expand(0x0068))); // addi a0, sp, 12 # addi4spn
    cr_assert(I_RV64I_SLLI == rv64_decode_instruction_tag(rv64c_expand(0x0506))); // slli a0, a0, 1
    cr_assert(I_RV64I_SRLI == rv64_decode_instruction_tag(rv64c_expand(0x8109))); // srli a0, a0, 2
    cr_assert(I_RV64I_SRAI == rv64_decode_instruction_tag(rv64c_expand(0x850d))); // srai a0, a0, 3
    cr_assert(I_RV64I_ANDI == rv64_decode_instruction_tag(rv64c_expand(0x893d))); // andi a0, a0, 15
    // 27.5.3 integer register-register operations
    //cr_assert(I_RV64I_MV == rv64_decode_instruction_tag(rv64c_expand(0x852e))); // mv a0, a1
    //cr_assert(I_RV64I_ADD == rv64_decode_instruction_tag(rv64c_expand(0x929a))); // add s0, s0, s1
    cr_assert(I_RV64I_AND == rv64_decode_instruction_tag(rv64c_expand(0x8c65))); // and s0, s0, s1
    cr_assert(I_RV64I_OR == rv64_decode_instruction_tag(rv64c_expand(0x8d4d))); // or a0, a0, a1
    cr_assert(I_RV64I_XOR == rv64_decode_instruction_tag(rv64c_expand(0x8e35))); // xor a2, a2, a3
    cr_assert(I_RV64I_SUB == rv64_decode_instruction_tag(rv64c_expand(0x8f1d))); // sub a4, a4, a5
    cr_assert(I_RV64I_ADDW == rv64_decode_instruction_tag(rv64c_expand(0x9c25))); // addw s0, s0, s1
    cr_assert(I_RV64I_SUBW == rv64_decode_instruction_tag(rv64c_expand(0x9c05))); // subw s0, s0, s1
    
    // 27.5.4 defined illegal instruction
    // 0
    // 27.5.5 nop instruction
    // C.NOP
    // 27.5.6 breakpoint instruction
    // C.EBREAK
    // 27.6 Usage of C instructions in LR/SC sequences
    // 27.7 HINT instructions
}
