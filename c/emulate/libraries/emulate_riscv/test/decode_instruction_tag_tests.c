#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_instructions.h"
#include "rv64/rv64_decode_instruction.h"

// MARK: Instructions
Test(emu_rv64_decode_tag_tests, decode_instruction_tests) {
    // MARK: RV64I
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

    cr_assert(I_RV64I_LB == rv64_decode_instruction_tag(0x00028303)); // lb t1, 0(t0)
    cr_assert(I_RV64I_LH == rv64_decode_instruction_tag(0x00029303)); // lh t1, 0(t0)
    cr_assert(I_RV64I_LW == rv64_decode_instruction_tag(0x0002a303)); // lw t1, 0(t0)
    cr_assert(I_RV64I_LBU == rv64_decode_instruction_tag(0x0002c303)); // lbu t1, 0(t0)
    cr_assert(I_RV64I_LHU == rv64_decode_instruction_tag(0x0002d303)); // lhu t1, 0(t0)
    cr_assert(I_RV64I_SB == rv64_decode_instruction_tag(0x00530023)); // sb t0, 0(t1)
    cr_assert(I_RV64I_SH == rv64_decode_instruction_tag(0x00531023)); // sh t0, 0(t1)
    cr_assert(I_RV64I_SW == rv64_decode_instruction_tag(0x00532023)); // sw t0, 0(t1)
    // "I" Type
    cr_assert(I_RV64I_ADDI == rv64_decode_instruction_tag(0x00550593)); // addi a1, a0, 5
    cr_assert(I_RV64I_SLTI == rv64_decode_instruction_tag(0x0003ae13)); // slti t3, t2, 0
    cr_assert(I_RV64I_SLTIU == rv64_decode_instruction_tag(0x0ff33293)); // sltiu t0, t1, 255
    cr_assert(I_RV64I_XORI == rv64_decode_instruction_tag(0x01034293)); // xori t0, t1, 16
    cr_assert(I_RV64I_ORI == rv64_decode_instruction_tag(0x020fef13)); // ori t5, t6, 32
    cr_assert(I_RV64I_ANDI == rv64_decode_instruction_tag(0x040efe13)); // andi t3, t4, 64
    cr_assert(I_RV64I_SLLI == rv64_decode_instruction_tag(0x00731293)); // slli t0, t0, 0xc
    cr_assert(I_RV64I_SRLI == rv64_decode_instruction_tag(0x00735293)); // srli t0, t0, 0xc
    cr_assert(I_RV64I_SRAI == rv64_decode_instruction_tag(0x40735293)); // srai t0, t0, 0xc

    // "R" Type
    cr_assert(I_RV64I_ADD == rv64_decode_instruction_tag(0x007302b3)); // add t0, t1, t2
    cr_assert(I_RV64I_SUB == rv64_decode_instruction_tag(0x41ff0eb3)); // sub t4, t5, t6
    cr_assert(I_RV64I_SLL == rv64_decode_instruction_tag(0x00629fb3)); // sll t6, t0, t1
    cr_assert(I_RV64I_SLT == rv64_decode_instruction_tag(0x007322b3)); // slt t0, t1, t2
    cr_assert(I_RV64I_SLTU == rv64_decode_instruction_tag(0x00c5b533)); // sltu a0, a1, a2
    cr_assert(I_RV64I_XOR == rv64_decode_instruction_tag(0x0093c533)); // xor a0, t2, s1
    cr_assert(I_RV64I_SRL == rv64_decode_instruction_tag(0x007352b3)); // srl t0, t1, t2
    cr_assert(I_RV64I_SRA == rv64_decode_instruction_tag(0x407352b3)); // sra t0, t1, t2
    cr_assert(I_RV64I_OR == rv64_decode_instruction_tag(0x007362b3)); // or t0, t1, t2
    cr_assert(I_RV64I_AND == rv64_decode_instruction_tag(0x007372b3)); // and t0, t1, t2
    cr_assert(I_RV64I_FENCE == rv64_decode_instruction_tag(0x0ff0000f)); // fence
    cr_assert(I_RV64I_FENCE_TSO == rv64_decode_instruction_tag(0x8330000f)); // fence.tso
    cr_assert(I_RV64I_ECALL == rv64_decode_instruction_tag(0x00000073)); // ecall
    cr_assert(I_RV64I_EBREAK == rv64_decode_instruction_tag(0x00100073)); // ebreak
    cr_assert(I_RV64I_LWU == rv64_decode_instruction_tag(0x0002e303)); // lwu t1, 0(t0)
    cr_assert(I_RV64I_LD == rv64_decode_instruction_tag(0x0002b303)); // ld t1, 0(t0)
    cr_assert(I_RV64I_SD == rv64_decode_instruction_tag(0x00533023)); // sd t0, 0(t1)
    cr_assert(I_RV64I_SLLI == rv64_decode_instruction_tag(0x00731293)); // slli t0, t1, 0x7
    cr_assert(I_RV64I_SRLI == rv64_decode_instruction_tag(0x00735293)); // srli t0, t1, 0x7
    cr_assert(I_RV64I_SRAI == rv64_decode_instruction_tag(0x40735293)); // srai t0, t1, 0x7
    cr_assert(I_RV64I_ADDIW == rv64_decode_instruction_tag(0x0073029b)); // addiw t0, t1, 7
    cr_assert(I_RV64I_SLLIW == rv64_decode_instruction_tag(0x0073129b)); // slliw t0, t1, 0x7
    cr_assert(I_RV64I_SRLIW == rv64_decode_instruction_tag(0x0073529b)); // srliw t0, t1, 0x7
    cr_assert(I_RV64I_SRAIW == rv64_decode_instruction_tag(0x4073529b)); // sraiw t0, t1, 0x7
    cr_assert(I_RV64I_ADDW == rv64_decode_instruction_tag(0x007302bb)); // addw t0, t1, t2
    cr_assert(I_RV64I_SUBW == rv64_decode_instruction_tag(0x407302bb)); // subw t0, t1, t2
    cr_assert(I_RV64I_SLLW == rv64_decode_instruction_tag(0x007312bb)); // sllw t0, t1, t2
    cr_assert(I_RV64I_SRLW == rv64_decode_instruction_tag(0x007352bb)); // srlw t0, t1, t2
    cr_assert(I_RV64I_SRAW == rv64_decode_instruction_tag(0x407352bb)); // sraw t0, t1, t2
    // I Extensions
    // RV64I Zihintpause
    cr_assert(I_RV64I_PAUSE == rv64_decode_instruction_tag(0x0100000f)); // pause
    // RV64I Zifencei
    cr_assert(I_RV64ZIFENCEI_FENCE_I == rv64_decode_instruction_tag(0x0000100f)); // fence.i
    // RV64I Zicsr
    cr_assert(I_RV64ZICSR_CSRRW == rv64_decode_instruction_tag(0x340292f3)); // csrrw t0, mscratch, t0
    cr_assert(I_RV64ZICSR_CSRRS == rv64_decode_instruction_tag(0x300322f3)); // csrrs t0, mstatus, t1
    cr_assert(I_RV64ZICSR_CSRRC == rv64_decode_instruction_tag(0x300332f3)); // csrrc t0, mstatus, t1
    cr_assert(I_RV64ZICSR_CSRRWI == rv64_decode_instruction_tag(0x3407d2f3)); // csrrwi t0, mscratch, 15
    cr_assert(I_RV64ZICSR_CSRRSI == rv64_decode_instruction_tag(0x3007e2f3)); // csrrsi t0, mstatus, 15
    cr_assert(I_RV64ZICSR_CSRRCI == rv64_decode_instruction_tag(0x3007f2f3)); // csrrci t0, mstatus, 15

    // MARK: RV64M
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
    // MARK: RV64A
    cr_assert(I_RV64ZALRSC_LR_W == rv64_decode_instruction_tag(0x100522af)); // lr.w t0, (a0)
    cr_assert(I_RV64ZALRSC_SC_W == rv64_decode_instruction_tag(0x18c522af)); // sc.w t0, a2, (a0)
    cr_assert(I_RV64ZAWRS_AMOSWAP_W == rv64_decode_instruction_tag(0x0805202f)); // amoswap.w t0, t1, (t2)
    cr_assert(I_RV64ZAWRS_AMOADD_W == rv64_decode_instruction_tag(0x0063a2af)); // amoadd.w t0, t1, (t2)
    cr_assert(I_RV64ZAWRS_AMOXOR_W == rv64_decode_instruction_tag(0x2063a2af)); // amoxor.w t0, t1, (t2)
    cr_assert(I_RV64ZAWRS_AMOAND_W == rv64_decode_instruction_tag(0x6063a2af)); // amoand.w t0, t1, (t2)
    cr_assert(I_RV64ZAWRS_AMOOR_W == rv64_decode_instruction_tag(0x4063a2af)); // amoor.w t0, t1, (t2)
    cr_assert(I_RV64ZAWRS_AMOMIN_W == rv64_decode_instruction_tag(0x8063a2af)); // amomin.w t0, t1, (t2)
    cr_assert(I_RV64ZAWRS_AMOMAX_W == rv64_decode_instruction_tag(0xa063a2af)); // amomax.w t0, t1, (t2)
    cr_assert(I_RV64ZAWRS_AMOMINU_W == rv64_decode_instruction_tag(0xc063a2af)); // amominu.w t0, t1, (t2)
    cr_assert(I_RV64ZAWRS_AMOMAXU_W == rv64_decode_instruction_tag(0xe063a2af)); // amomaxu.w t0, t1, (t2)
    cr_assert(I_RV64ZALRSC_LR_D == rv64_decode_instruction_tag(0x100532af)); // lr.d t0, (a0)
    cr_assert(I_RV64ZALRSC_SC_D == rv64_decode_instruction_tag(0x18c532af)); // sc.d t0, a2, (a0)
    cr_assert(I_RV64ZAWRS_AMOSWAP_D == rv64_decode_instruction_tag(0x0863b2af)); // amoswap.d t0, t1, (t2)
    cr_assert(I_RV64ZAWRS_AMOADD_D == rv64_decode_instruction_tag(0x0063b2af)); // amoadd.d t0, t1, (t2)
    cr_assert(I_RV64ZAWRS_AMOXOR_D == rv64_decode_instruction_tag(0x2063b2af)); // amoxor.d t0, t1, (t2)
    cr_assert(I_RV64ZAWRS_AMOAND_D == rv64_decode_instruction_tag(0x6063b2af)); // amoand.d t0, t1, (t2)
    cr_assert(I_RV64ZAWRS_AMOOR_D == rv64_decode_instruction_tag(0x4063b2af)); // amoor.d t0, t1, (t2)
    cr_assert(I_RV64ZAWRS_AMOMIN_D == rv64_decode_instruction_tag(0x8063b2af)); // amomin.d t0, t1, (t2)
    cr_assert(I_RV64ZAWRS_AMOMAX_D == rv64_decode_instruction_tag(0xa063b2af)); // amomax.d t0, t1, (t2)
    cr_assert(I_RV64ZAWRS_AMOMINU_D == rv64_decode_instruction_tag(0xc063b2af)); // amominu.d t0, t1, (t2)
    cr_assert(I_RV64ZAWRS_AMOMAXU_D == rv64_decode_instruction_tag(0xe063b2af)); // amomaxu.d t0, t1, (t2)

    // ...
    // MARK: RV64F
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
    // MARK: RV64V
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
