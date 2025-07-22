#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_instructions.h"
#include "rv64/rv64_decode_instruction.h"

// MARK: Instructions
Test(emu_rv64_decode_tag_tests, decode_instruction_tests) {
    // RV64I
    cr_assert(I_RV64I_LUI == rv64_decode_instruction_tag(0x123452b7)); // lui t0, 0x12345
    cr_assert(I_RV64I_AUIPC == rv64_decode_instruction_tag(0x12345297)); // auipc t0, 0x12345
    cr_assert(I_RV64I_JAL == rv64_decode_instruction_tag(0xf71ff0ef)); // jal ra, symbol
    cr_assert(I_RV64I_JALR == rv64_decode_instruction_tag(0x00008067)); // jalr zero, ra, 0
    // "B" Type - Branch
    cr_assert(I_RV64I_BEQ == rv64_decode_instruction_tag(0xfe058ce3)); // beq a1, x0, loop_start
    cr_assert(I_RV64I_BNE == rv64_decode_instruction_tag(0xfe059ae3)); // bne a1, x0, loop_start
    cr_assert(I_RV64I_BLT == rv64_decode_instruction_tag(0xfe05c8e3)); // blt a1, x0, loop_start
    cr_assert(I_RV64I_BGE == rv64_decode_instruction_tag(0xfe05d4e3)); // bge a1, x0, loop_start
    cr_assert(I_RV64I_BLTU == rv64_decode_instruction_tag(0xfe05e6e3)); // bltu a1, x0, loop_start
    cr_assert(I_RV64I_BGEU == rv64_decode_instruction_tag(0xfe05f2e3)); // bgeu a1, x0, loop_start
    // TODO: LB - SW
    // ...
    // "I" Type
    cr_assert(I_RV64I_ADDI == rv64_decode_instruction_tag(0x00550593)); // addi a1, a0, 5
    cr_assert(I_RV64I_ADD == rv64_decode_instruction_tag(0x007302b3)); // add t0, t1, t2
    cr_assert(I_RV64I_SLTI == rv64_decode_instruction_tag(0x0003ae13)); // slti t3, t2, 0
    cr_assert(I_RV64I_SLTIU == rv64_decode_instruction_tag(0x0ff33293)); // sltiu t0, t1, 255
    cr_assert(I_RV64I_XORI == rv64_decode_instruction_tag(0x01034293)); // xori t0, t1, 16
    cr_assert(I_RV64I_ORI == rv64_decode_instruction_tag(0x020fef13)); // ori t5, t6, 32
    cr_assert(I_RV64I_ANDI == rv64_decode_instruction_tag(0x040efe13)); // andi t3, t4, 64
    // "R" Type
    cr_assert(I_RV64I_ADD == rv64_decode_instruction_tag(0x007302b3)); // add t0, t1, t2
    cr_assert(I_RV64I_SUB == rv64_decode_instruction_tag(0x41ff0eb3)); // sub t4, t5, t6
    cr_assert(I_RV64I_SLL == rv64_decode_instruction_tag(0x00629fb3)); // sll t6, t0, t1
    cr_assert(I_RV64I_SLT == rv64_decode_instruction_tag(0x007322b3)); // slt t0, t1, t2
    cr_assert(I_RV64I_SLTU == rv64_decode_instruction_tag(0x00c5b533)); // sltu a0, a1, a2
    cr_assert(I_RV64I_XOR == rv64_decode_instruction_tag(0x0093c533)); // xor a0, t2, s1
    // ...
    // I Extensions
    // RV64I Zifencei

    // RV64I Zicsr

    // RV64M
    cr_assert(I_RV64M_MUL == rv64_decode_instruction_tag(0x027302b3)); // mul t0, t1, t2
    cr_assert(I_RV64M_MULH == rv64_decode_instruction_tag(0x027312b3)); // mulh t0, t1, t2
    cr_assert(I_RV64M_MULHSU == rv64_decode_instruction_tag(0x027322b3)); // mulhsu t0, t1, t2
    cr_assert(I_RV64M_MULHU == rv64_decode_instruction_tag(0x027332b3)); // mulhu t0, t1, t2
    cr_assert(I_RV64M_DIV == rv64_decode_instruction_tag(0x027342b3)); // div t0, t1, t2
    cr_assert(I_RV64M_DIVU == rv64_decode_instruction_tag(0x027352b3)); // divu t0, t1, t2
    cr_assert(I_RV64M_REM == rv64_decode_instruction_tag(0x027362b3)); // rem t0, t1, t2
    cr_assert(I_RV64M_REMU == rv64_decode_instruction_tag(0x027372b3)); // remu t0, t1, t2
    cr_assert(I_RV64M_MULW == rv64_decode_instruction_tag(0x027302bb)); // mulw t0, t1, t2
    cr_assert(I_RV64M_DIVW == rv64_decode_instruction_tag(0x027342bb)); // divw t0, t1, t2
    cr_assert(I_RV64M_DIVUW == rv64_decode_instruction_tag(0x027352bb)); // divuw t0, t1, t2
    cr_assert(I_RV64M_REMW == rv64_decode_instruction_tag(0x027362bb)); // remw t0, t1, t2
    cr_assert(I_RV64M_REMUW == rv64_decode_instruction_tag(0x027372bb)); // remuw t0, t1, t2
    // RV64A
    cr_assert(I_RV64ZALRSC_LR_W == rv64_decode_instruction_tag(0x100522af)); // lr.w t0, (a0)
    cr_assert(I_RV64ZALRSC_SC_W == rv64_decode_instruction_tag(0x18c522af)); // sc.w t0, a2, (a0)
    cr_assert(I_RV64ZAWRS_AMOSWAP_W == rv64_decode_instruction_tag(0x0805202f)); // amoswap.w x0, x0, (a0)
    // ...
    // RV64F
    cr_assert(I_RV64F_FLW == rv64_decode_instruction_tag(0x0009a187)); // flw ft3, (s3)
    cr_assert(I_RV64F_FSW == rv64_decode_instruction_tag(0x0039a027)); // fsw ft3, (s3)
    cr_assert(I_RV64F_FMADD_S == rv64_decode_instruction_tag(0x60b57543)); // fmadd.s fa0, fa0, fa1, fa2
    cr_assert(I_RV64F_FMSUB_S == rv64_decode_instruction_tag(0x60b57547)); // fmsub.s fa0, fa0, fa1, fa2
    cr_assert(I_RV64F_FNMSUB_S == rv64_decode_instruction_tag(0x60b5754b)); // fnmsub.s fa0, fa0, fa1, fa2
    cr_assert(I_RV64F_FNMADD_S == rv64_decode_instruction_tag(0x60b5754f)); // fnmadd.s fa0, fa0, fa1, fa2
    cr_assert(I_RV64F_FADD_S == rv64_decode_instruction_tag(0x00b57553)); // fadd.s fa0, fa0, fa1
    cr_assert(I_RV64F_FSUB_S == rv64_decode_instruction_tag(0x08b57553)); // fsub.s fa0, fa0, fa1
    cr_assert(I_RV64F_FMUL_S == rv64_decode_instruction_tag(0x10b57553)); // fmul.s fa0, fa0, fa1
    cr_assert(I_RV64F_FDIV_S == rv64_decode_instruction_tag(0x18b57553)); // fdiv.s fa0, fa0, fa1
    cr_assert(I_RV64F_FSQRT_S == rv64_decode_instruction_tag(0x58057553)); // fsqrt.s fa0, fa0
    cr_assert(I_RV64F_FSGNJ_S == rv64_decode_instruction_tag(0x20b50553)); // fsgnj.s fa0, fa0, fa1
    cr_assert(I_RV64F_FSGNJN_S == rv64_decode_instruction_tag(0x20b51553)); // fsgnjn.s fa0, fa0, fa1
    cr_assert(I_RV64F_FSGNJX_S == rv64_decode_instruction_tag(0x20b52553)); // fsgnjx.s fa0, fa0, fa1
    cr_assert(I_RV64F_FMIN_S == rv64_decode_instruction_tag(0x28b50553)); // fmin.s fa0, fa0, fa1
    cr_assert(I_RV64F_FMAX_S == rv64_decode_instruction_tag(0x28b51553)); // fmax.s fa0, fa0, fa1
    cr_assert(I_RV64F_FCVT_W_S == rv64_decode_instruction_tag(0xc0057553)); // fcvt.w.s a0, fa0
    cr_assert(I_RV64F_FCVT_WU_S == rv64_decode_instruction_tag(0xc0157553)); // fcvt.wu.s a0, fa0
    cr_assert(I_RV64F_FMV_X_W == rv64_decode_instruction_tag(0xe0050553)); // fmv.x.w a0, fa0
    cr_assert(I_RV64F_FEQ_S == rv64_decode_instruction_tag(0xa0b52553)); // feq.s a0, fa0, fa1
    cr_assert(I_RV64F_FLT_S == rv64_decode_instruction_tag(0xa0b51553)); // flt.s a0, fa0, fa1
    cr_assert(I_RV64F_FLE_S == rv64_decode_instruction_tag(0xa0b50553)); // fle.s a0, fa0, fa1
    cr_assert(I_RV64F_FCLASS_S == rv64_decode_instruction_tag(0xe0051553)); // fclass.s a0, fa0
    cr_assert(I_RV64F_FCVT_S_W == rv64_decode_instruction_tag(0xd0057553)); // fcvt.s.w fa0, a0
    cr_assert(I_RV64F_FCVT_S_WU == rv64_decode_instruction_tag(0xd0157553)); // fcvt.s.wu fa0, a0
    cr_assert(I_RV64F_FMV_W_X == rv64_decode_instruction_tag(0xf0050553)); // fmv.w.x fa0, a0
    cr_assert(I_RV64F_FCVT_L_S == rv64_decode_instruction_tag(0xc0257553)); // fcvt.l.s a0, fa0
    cr_assert(I_RV64F_FCVT_LU_S == rv64_decode_instruction_tag(0xc0357553)); // fcvt.lu.s a0, fa0
    cr_assert(I_RV64F_FCVT_S_L == rv64_decode_instruction_tag(0xd0257553)); // fcvt.s.l fa0, a0
    cr_assert(I_RV64F_FCVT_S_LU == rv64_decode_instruction_tag(0xd0357553)); // fcvt.s.lu fa0, a0
    // RV64D
    // RV64Q
    // ...
    // RV64V
    // vector load/store
    cr_assert(I_RV64V_VSETVLI == rv64_decode_instruction_tag(0x0d06f757)); // vsetvli a4, a3, e32, m1, ta, ma
    cr_assert(I_RV64V_VSETIVLI == rv64_decode_instruction_tag(0xc804f2d7)); // vsetivli t0, 9, e8, m1, tu, ma
    cr_assert(I_RV64V_VSETVL == rv64_decode_instruction_tag(0x81fe7ed7)); // vsetvl t4, t3, t6
    cr_assert(I_RV64V_VLE64_V == rv64_decode_instruction_tag(0x0205f087)); // vle64.v v0, (a0)
    cr_assert(I_RV64V_VLE32_V == rv64_decode_instruction_tag(0x02056007)); // vle32.v v0, (a0)
    cr_assert(I_RV64V_VLE16_V == rv64_decode_instruction_tag(0x0205d087)); // vle16.v v0, (a0)
    cr_assert(I_RV64V_VLE8_V == rv64_decode_instruction_tag(0x02058087)); // vle8.v v0, (a0)
    // todo: vls, vlx?
    cr_assert(I_RV64V_VSE64_V == rv64_decode_instruction_tag(0x02057027)); // vse64.v v0, (a0)
    cr_assert(I_RV64V_VSE32_V == rv64_decode_instruction_tag(0x02056027)); // vse32.v v0, (a0)
    cr_assert(I_RV64V_VSE16_V == rv64_decode_instruction_tag(0x02055027)); // vse16.v v0, (a0)
    cr_assert(I_RV64V_VSE8_V == rv64_decode_instruction_tag(0x02050027)); // vse8.v v0, (a0)
    // todo: vss, vsx?
    // vector arithmetic (add/sub/mul/div/rem/multiply-add)
    cr_assert(I_RV64V_VADD_VV == rv64_decode_instruction_tag(0x02110057)); // vadd.vv v0, v1, v2
    cr_assert(I_RV64V_VADD_VX == rv64_decode_instruction_tag(0x0215c057)); // vadd.vx v0, v1, a1
    cr_assert(I_RV64V_VADD_VI == rv64_decode_instruction_tag(0x0213b057)); // vadd.vi v0, v1, 7
    cr_assert(I_RV64V_VSUB_VV == rv64_decode_instruction_tag(0x0a110057)); // vsub.vv v0, v1, v2
    cr_assert(I_RV64V_VSUB_VX == rv64_decode_instruction_tag(0x0a15c057)); // vsub.vx v0, v1, a1
    cr_assert(I_RV64V_VRSUB_VX == rv64_decode_instruction_tag(0x0e15c057)); // vrsub.vx v0, v1, a1
    cr_assert(I_RV64V_VRSUB_VI == rv64_decode_instruction_tag(0x0e13b057)); // vrsub.vi v0, v1, 7
    // vector widening/narrowing
    // vector boolean (and/or/xor/not)
    //
    // todo: add? sub? mul? etc?

}
