#include <criterion/criterion.h>

#include "rv64/rv64_instructions.h"
#include "rv64/rv64_decode_instruction.h"
#include "rv64/modules/rv64c_compressed.h"

Test(emu_rv64c_expand_tag_tests, expand_instruction_tests) {
    // MARK: Q0
    cr_assert(I_RV64I_LW == rv64_decode_instruction_tag(rv64c_expand(0x41c8))); // lw a0, 4(a1)
    cr_assert(I_RV64I_LD == rv64_decode_instruction_tag(rv64c_expand(0x6588))); // ld a0, 8(a1)
    cr_assert(I_RV64I_SW == rv64_decode_instruction_tag(rv64c_expand(0xc1c8))); // sw a0, 4(a1)
    cr_assert(I_RV64I_SD == rv64_decode_instruction_tag(rv64c_expand(0xe588))); // sd a0, 8(a1)
    // RV64D not implemented yet
    // TODO - fld (rv64d - not implemented yet)
    // TODO - fsd (rv64d - not implemented yet)

    // MARK: Q1
    // TODO - C.J
    //cr_assert(I_RV64I_JAL == rv64_decode_instruction_tag(rv64c_expand(0xa031))); // j .+12
    // TODO - C.BEQZ
    //cr_assert(I_RV64I_BNEZ == rv64_decode_instruction_tag(rv64c_expand(0xa031))); // bnez a0, 20 
    cr_assert(I_RV64I_ADDI == rv64_decode_instruction_tag(rv64c_expand(0x557d))); // li a0, -1
    cr_assert(I_RV64I_LUI == rv64_decode_instruction_tag(rv64c_expand(0x6291))); // lui t0, 0x4
    cr_assert(I_RV64I_ADDI == rv64_decode_instruction_tag(rv64c_expand(0x0341))); // addi t1, t1, 16
    cr_assert(I_RV64I_ADDIW == rv64_decode_instruction_tag(rv64c_expand(0x2341))); // addiw t1, t1, 16
    //cr_assert(I_RV64I_ADDI == rv64_decode_instruction_tag(rv64c_expand(0x1101))); // addi sp, sp, -32 # addi16sp
    //cr_assert(I_RV64I_ADDI == rv64_decode_instruction_tag(rv64c_expand(0x0068))); // addi a0, sp, 12 # addi4spn
    cr_assert(I_RV64I_SRLI == rv64_decode_instruction_tag(rv64c_expand(0x8109))); // srli a0, a0, 2
    cr_assert(I_RV64I_SRAI == rv64_decode_instruction_tag(rv64c_expand(0x850d))); // srai a0, a0, 3
    cr_assert(I_RV64I_ANDI == rv64_decode_instruction_tag(rv64c_expand(0x893d))); // andi a0, a0, 15
    cr_assert(I_RV64I_AND == rv64_decode_instruction_tag(rv64c_expand(0x8c65))); // and s0, s0, s1
    cr_assert(I_RV64I_OR == rv64_decode_instruction_tag(rv64c_expand(0x8d4d))); // or a0, a0, a1
    cr_assert(I_RV64I_XOR == rv64_decode_instruction_tag(rv64c_expand(0x8e35))); // xor a2, a2, a3
    cr_assert(I_RV64I_SUB == rv64_decode_instruction_tag(rv64c_expand(0x8f1d))); // sub a4, a4, a5
    cr_assert(I_RV64I_ADDW == rv64_decode_instruction_tag(rv64c_expand(0x9c25))); // addw s0, s0, s1
    cr_assert(I_RV64I_SUBW == rv64_decode_instruction_tag(rv64c_expand(0x9c05))); // subw s0, s0, s1
    // TODO - 0
    // TODO - C.NOP

    // MARK: Q2
    cr_assert(I_RV64I_SLLI == rv64_decode_instruction_tag(rv64c_expand(0x0506))); // slli a0, a0, 1
    cr_assert(I_RV64I_LW == rv64_decode_instruction_tag(rv64c_expand(0x4532))); // lw a0, 12(sp) # lwsp
    cr_assert(I_RV64I_LD == rv64_decode_instruction_tag(rv64c_expand(0x64c2))); // ld s1, 16(sp) # ldsp
    // TODO - FLDSP (rv64d - not implemented yet)
    cr_assert(I_RV64I_JALR == rv64_decode_instruction_tag(rv64c_expand(0x8082))); // jr ra # c.jr
    cr_assert(I_RV64I_ADD == rv64_decode_instruction_tag(rv64c_expand(0x852e))); // c.mv a0, a1 # c.mv
    cr_assert(I_RV64I_EBREAK == rv64_decode_instruction_tag(rv64c_expand(0x9002))); // ebreak
    cr_assert(I_RV64I_JALR == rv64_decode_instruction_tag(rv64c_expand(0x9082))); // jalr a0
    cr_assert(I_RV64I_ADD == rv64_decode_instruction_tag(rv64c_expand(0x929a))); // add s0, s0, s1
    cr_assert(I_RV64I_SW == rv64_decode_instruction_tag(rv64c_expand(0xc62a))); // sw a0, 12(sp) # swsp
    // TODO - FSDSP (rv64d - not implemented yet)
    cr_assert(I_RV64I_SD == rv64_decode_instruction_tag(rv64c_expand(0xe826))); // sd s1, 16(sp) # sdsp
}
