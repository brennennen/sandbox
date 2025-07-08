#ifndef INSTRUCTIONS_RV64_H
#define INSTRUCTIONS_RV64_H

#include <stdint.h>
#include <inttypes.h>

#include "shared/include/binary_utilities.h"

// MARK: Instructions
/**
 * All riscv5 instructions supported by this emulator.
 *
 * @warning: Parallal array with "rv64_instruction_tag_mnemonic".
 */
typedef enum ENUM_PACK_ATTRIBUTE {
    I_RV64_INVALID,

    // MARK: RV64I - Integer
    I_RV64I_LUI, /** Load Upper Immediate */
    I_RV64I_AUIPC, /** Add Upper Immediate to pc */
    I_RV64I_JAL, /** Jump and Link */
    I_RV64I_JALR, /** Jump and Link and Return */
    I_RV64I_BEQ,
    I_RV64I_BNE,
    I_RV64I_BLT,
    I_RV64I_BGE,
    I_RV64I_BLTU,
    I_RV64I_BGEU,
    I_RV64I_LB, /** Load Byte - Loads 8 bit value, then sign-extends to 32-bits before storing in rd.*/
    I_RV64I_LH, /** Load Half Word - Loads 16 bit value, then sign-extends to 32-bits before storing in rd. */
    I_RV64I_LW, /** Load Word - Loads a 32-bit value from memory into rd. */
    I_RV64I_LBU, /** Load Byte Unsigned - Loads 8 bit value, then 0 extends to 32-bits before storing in rd.*/
    I_RV64I_LHU, /** Load Half Word Unsigned - Loads 16 bit value, then 0 extends to 32-bits before storing in rd. */
    I_RV64I_SB, /** Store Byte */
    I_RV64I_SH, /** Store Half Word*/
    I_RV64I_SW, /** Store Word */
    I_RV64I_ADDI, /** Add Immediate */
    I_RV64I_SLTI, /** Set Less Than Immediate */
    I_RV64I_SLTIU, /** Set Less Than Immediate Unsigned */
    I_RV64I_XORI, /** XOR Immediate */
    I_RV64I_ORI,
    I_RV64I_ANDI,
    I_RV64I_SLLI,
    I_RV64I_SRLI,
    I_RV64I_SRAI,
    I_RV64I_ADD,
    I_RV64I_SUB,
    I_RV64I_SLL,
    I_RV64I_SLT,
    I_RV64I_SLTU,
    I_RV64I_XOR,
    I_RV64I_SRL,
    I_RV64I_SRA,
    I_RV64I_OR,
    I_RV64I_AND,
    I_RV64I_FENCE, /** Memory/Core Fencing - TODO: is this part of RV64I or in it's own Zifencei? */
    I_RV64I_FENCE_TSO,
    I_RV64I_PAUSE,
    I_RV64I_ECALL,
    I_RV64I_EBREAK,
    I_RV64I_LWU,
    I_RV64I_LD,
    I_RV64I_SD,
    I_RV64I_ADDIW,
    I_RV64I_SLLIW,
    I_RV64I_SRLIW,
    I_RV64I_SRAIW,
    I_RV64I_ADDW,
    I_RV64I_SUBW,
    I_RV64I_SLLW,
    I_RV64I_SRLW,
    I_RV64I_SRAW,

    /*
     * MARK: RV64Zifencei
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#zifencei
     */
    I_RV64ZIFENCEI_FENCE_I,

    /*
     * MARK: RV64Zicsr
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#csrinsts
     */
    I_RV64ZICSR_CSRRW,
    I_RV64ZICSR_CSRRS,
    I_RV64ZICSR_CSRRC,
    I_RV64ZICSR_CSRRWI,
    I_RV64ZICSR_CSRRSI,
    I_RV64ZICSR_CSRRCI,

    // MARK: RV64M - Multiplication
    I_RV64M_MUL, /** Multiply - Multiply 2 64-bit values together. */
    I_RV64M_MULH, /** Multiply High - Multiply 2 64-bit values together, interprets the results of 128 bits and returns the upper/high 64 bits. */
    I_RV64M_MULHSU, /** ??? */
    I_RV64M_MULHU,
    I_RV64M_DIV,
    I_RV64M_DIVU,
    I_RV64M_REM,
    I_RV64M_REMU,
    I_RV64M_MULW,
    I_RV64M_DIVW,
    I_RV64M_DIVUW,
    I_RV64M_REMW,
    I_RV64M_REMUW,

    // MARK: RV64A - Atomic
    I_RV64A_LR_W, /** Load-Reserved Word */
    I_RV64A_SC_W, /** Store-Conditional */
    I_RV64A_AMOSWAP_W, /** Atomic Swap */
    I_RV64A_AMOADD_W,
    I_RV64A_AMOXOR_W,
    I_RV64A_AMOAND_W,
    I_RV64A_AMOOR_W,
    I_RV64A_AMOMIN_W,
    I_RV64A_AMOMAX_W,
    I_RV64A_AMOMINU_W,
    I_RV64A_AMOMAXU_W,
    I_RV64A_LR_D,
    I_RV64A_SC_D,
    I_RV64A_AMOSWAP_D,
    I_RV64A_AMOADD_D,
    I_RV64A_AMOXOR_D,
    I_RV64A_AMOAND_D,
    I_RV64A_AMOOR_D,
    I_RV64A_AMOMIN_D,
    I_RV64A_AMOMAX_D,
    I_RV64A_AMOMINU_D,
    I_RV64A_AMOMAXU_D,

    // MARK: RV64F - Floating Point Arithmetic (single word, 32 bit)
    I_RV64F_FLW, /** Float Load Word */
    I_RV64F_FSW, /** Float Store Word */
    I_RV64F_FMADD_S, /** Float Fused Multiply Add Single-Precision */
    I_RV64F_FMSUB_S, /** Float Fused Multiply Subtract Single-Precision */
    I_RV64F_FNMSUB_S, /** Float Fused Negative Multiply Subtract Single-Precision - Multiplies two numbers, subtracts a third, then negates the final result. */
    I_RV64F_FNMADD_S,
    I_RV64F_FADD_S, /** Float Add Single-Precision */
    I_RV64F_FSUB_S, /** Float Subtract Single-Precision */
    I_RV64F_FMUL_S, /** Float Multiply Single-Precision */
    I_RV64F_FDIV_S, /** Float Divide Single-Precision */
    I_RV64F_FSQRT_S, /** Float Square Root Single-Precision */
    I_RV64F_FSGNJ_S, /** Float SiGN inJect Single-Precision - Copies sign of one float to another float (cstd: copysignf).*/
    I_RV64F_FSGNJN_S, /** Float SiGN inJect Negated Single-Precision */
    I_RV64F_FSGNJX_S, /** Float SiGN inJect Xor Single-Precision */
    I_RV64F_FMIN_S,
    I_RV64F_FMAX_S,
    I_RV64F_FCVT_W_S, /** Float ConVerT Word Single-Precision - Converts a float to a 32-bit signed int. */
    I_RV64F_FCVT_WU_S, /** Float ConVerT Word Single-Precision - Converts a float to a 32-bit unsigned int. */
    I_RV64F_FMV_X_W, /** Float Move Word to "X" Single-Precision */
    I_RV64F_FEQ_S, /** Float EQuals Single-Precision - Checks if two floats are equal or not. */
    I_RV64F_FLT_S, /** Float Less Than Single-Precision */
    I_RV64F_FLE_S, /** Float Less Than or Equal To Single-Precision */
    I_RV64F_FCLASS_S, /** Float CLASSify Single-Precision */
    I_RV64F_FCVT_S_W, /** Float ConVerT Single-Precision to Word */
    I_RV64F_FCVT_S_WU, /** Float ConVerT Single-Precision to Word Unsigned */
    I_RV64F_FMV_W_X, /** Float Move "X" Single-Precision to Word */
    I_RV64F_FCVT_L_S, /** Float ConVerT Long to Single-Precision */
    I_RV64F_FCVT_LU_S, /** Float ConVerT Long Unsigned to Single-Precision */
    I_RV64F_FCVT_S_L, /** Float ConVerT Single-Precision to Long */
    I_RV64F_FCVT_S_LU, /** Float ConVerT Single-Precision to Long Unsigned */

    // MARK: RV64D - Double-Word (64 bit)
    I_RV64D_FLD,
    I_RV64D_FSD,
    I_RV64D_FMADD_D,
    I_RV64D_FMSUB_D,
    I_RV64D_FNMSUB_D,
    I_RV64D_FNMADD_D,
    I_RV64D_FADD_D,
    I_RV64D_FSUB_D,
    I_RV64D_FMUL_D,
    I_RV64D_FDIV_D,
    I_RV64D_FSQRT_D,
    I_RV64D_FSGNJ_D,
    I_RV64D_FSGNJN_D,
    I_RV64D_FSGNJX_D,
    I_RV64D_FMIN_D,
    I_RV64D_FMAX_D,
    I_RV64D_FCVT_S_D,
    I_RV64D_FCVT_D_S,
    I_RV64D_FEQ_D,
    I_RV64D_FLT_D,
    I_RV64D_FLE_D,
    I_RV64D_FCLASS_D,
    I_RV64D_FCVT_W_D,
    I_RV64D_FCVT_WU_D,
    I_RV64D_FCVT_D_W,
    I_RV64D_FCVT_D_WU,
    I_RV64D_FCVT_L_D,
    I_RV64D_FCVT_LU_D,
    I_RV64D_FMV_X_D,
    I_RV64D_FCVT_D_L,
    I_RV64D_FCVT_D_LU,
    I_RV64D_FMV_D_X,

    // MARK: RV64Q - Quad-word (128 bit)
    I_RV64Q_FLQ,
    I_RV64Q_FSQ,
    I_RV64Q_FMADD_Q,
    I_RV64Q_FMSUB_Q,
    I_RV64Q_FNMSUB_Q,
    I_RV64Q_FNMADD_Q,
    I_RV64Q_FADD_Q,
    I_RV64Q_FSUB_Q,
    I_RV64Q_FMUL_Q,
    I_RV64Q_FDIV_Q,
    I_RV64Q_FSQRT_Q,
    I_RV64Q_FSGNJ_Q,
    I_RV64Q_FSGNJN_Q,
    I_RV64Q_FSGNJX_Q,
    I_RV64Q_FMIN_Q,
    I_RV64Q_FMAX_Q,
    I_RV64Q_FCVT_S_Q,
    I_RV64Q_FCVT_Q_S,
    I_RV64Q_FCVT_D_Q,
    I_RV64Q_FCVT_Q_D,
    I_RV64Q_FEQ_Q,
    I_RV64Q_FLT_Q,
    I_RV64Q_FLE_Q,
    I_RV64Q_FCLASS_Q,
    I_RV64Q_FCVT_W_Q,
    I_RV64Q_FCVT_WU_Q,
    I_RV64Q_FCVT_Q_W,
    I_RV64Q_FCVT_Q_WU,
    I_RV64Q_FCVT_L_Q,
    I_RV64Q_FCVT_LU_Q,
    I_RV64Q_FCVT_Q_L,
    I_RV64Q_FCVT_Q_LU,

    // MARK: RV64Zfh - Half-word (16 bit)
    I_RV64ZFH_FLH,
    I_RV64ZFH_FSH,
    I_RV64ZFH_FMADD_H,
    I_RV64ZFH_FMSUB_H,
    I_RV64ZFH_FNMSUB_H,
    I_RV64ZFH_FNMADD_H,
    I_RV64ZFH_FADD_H,
    I_RV64ZFH_FSUB_H,
    I_RV64ZFH_FMUL_H,
    I_RV64ZFH_FDIV_H,
    I_RV64ZFH_FSQRT_H,
    I_RV64ZFH_FSGNJ_H,
    I_RV64ZFH_FSGNJN_H,
    I_RV64ZFH_FSGNJX_H,
    I_RV64ZFH_FMIN_H,
    I_RV64ZFH_FMAX_H,
    I_RV64ZFH_FCVT_S_H,
    I_RV64ZFH_FCVT_H_S,
    I_RV64ZFH_FCVT_D_H,
    I_RV64ZFH_FCVT_H_D,
    I_RV64ZFH_Q_H,
    I_RV64ZFH_H_Q,
    I_RV64ZFH_FEQ_H,
    I_RV64ZFH_FLT_H,
    I_RV64ZFH_FLE_H,
    I_RV64ZFH_FCLASS_H,
    I_RV64ZFH_FCVT_W_H,
    I_RV64ZFH_FCVT_WU_H,
    I_RV64ZFH_X_H,
    I_RV64ZFH_FCVT_H_W,
    I_RV64ZFH_FCVT_H_WU,
    I_RV64ZFH_FMV_H_X,
    I_RV64ZFH_FCVT_L_H,
    I_RV64ZFH_FCVT_LU_H,
    I_RV64ZFH_FCVT_H_L,
    I_RV64ZFH_FCVT_H_LU,

    // MARK: Zawrs
    I_RV64ZAWRS_WRS_NTO,
    I_RV64ZAWRS_WRS_STO,

    // MARK: RV64Zv*. - Vector Computation
    // MARK:
    //I_RV64V_


} instruction_tag_rv64_t;

/**
 * Lookup table for machine instructions to their associated assembly instruction
 * mnemonics.
 * @warning: Parallal array with "instruction_tag_rv64_t".
 */
static char rv64_instruction_tag_mnemonic[][16] = {
    "invalid",
    // RV64I
    "lui",
    "auipc",
    "jal",
    "jalr",
    "beq",
    "bne",
    "blt",
    "bge",
    "bltu",
    "bgeu",
    "lb",
    "lh",
    "lw",
    "lbu",
    "lhu",
    "sb",
    "sh",
    "sw",
    "addi",
    "slti",
    "sltui",
    "xori",
    "ori",
    "andi",
    "slli",
    "srli",
    "srai",
    "add",
    "sub",
    "sll",
    "slt",
    "sltu",
    "xor",
    "srl",
    "sra",
    "or",
    "and",
    "fence",
    "fence.tso",
    "pause",
    "ecall",
    "ebreak",
    "lwu",
    "ld",
    "sd",
    "addiw",
    "slliw",
    "srliw",
    "sraiw",
    "addw",
    "subw",
    "sllw",
    "srlw",
    "sraw",
    // RV64Zifencei
    "fence.i",
    // RV64Zicsr
    "csrrw",
    "csrrs",
    "csrrc",
    "csrrwi",
    "csrrsi",
    "csrrci",
    // RV64M
    "mul",
    "mulh",
    "mulhsu",
    "mulhu",
    "div",
    "divu",
    "rem",
    "remu",
    "mulw",
    "divw",
    "divuw",
    "remw",
    "remuw",
    // RV64A
    "lr.w",
    "sc.w",
    "amoswap.w",
    "amoadd.w",
    "amoxor.w",
    "amoand.w",
    "amoor.w",
    "amomin.w",
    "amomax.w",
    "amominu.w",
    "amomaxu.w",
    "lr.d",
    "sc.d",
    "amoswap.d",
    "amoadd.d",
    "amoxor.d",
    "amoand.d",
    "amoor.d",
    "amomin.d",
    "amomax.d",
    "amominu.d",
    "amomaxu.d",
    // RV64F
    "flw",
    "fsw",
    "fmadd.s",
    "fmsub.s",
    "fnmsub.s",
    "fnmadd.s",
    "fadd.s",
    "fsub.s",
    "fmul.s",
    "fdiv.s",
    "fsqrt.s",
    "fsgnj.s",
    "fsgnjn.s",
    "fsgnjx.s",
    "fmin.s",
    "fmax.s",
    "fcvt.w.s",
    "fcvt.wu.s",
    "fmv.x.w",
    "feq.s",
    "flt.s",
    "fle.s",
    "fclass.s",
    "fcvt.s.w",
    "fcvt.s.wu",
    "fmv.w.x",
    "fcvt.l.s",
    "fcvt.lu.s",
    "fcvt.s.l",
    "fcvt.s.lu",
    // RV64D
    "fld",
    "fsd",
    "fmadd.d",
    "fmsub.d",
    "fnmsub.d",
    "fnmadd.d",
    "fadd.d",
    "fsub.d",
    "fmul.d",
    "fdiv.d",
    "fsqrt.d",
    "fsgnj.d",
    "fsgnjn.d",
    "fsgnjx.d",
    "fmin.d",
    "fmax.d",
    "fcvt.s.d",
    "fcvt.d.s",
    "feq.d",
    "flt.d",
    "fle.d",
    "fclass.d",
    "fcvt.w.d",
    "fcvt.wu.d",
    "fcvt.d.w",
    "fcvt.d.wu",
    "fcvt.l.d",
    "fcvt.lu.d",
    "fmv.x.d",
    "fcvt.d.l",
    "fcvt.d.lu",
    "fmv.d.x",
    // RV64Q
    "flq",
    "fsq",
    "fmadd.q",
    "fmsub.q",
    "fnmsub.q",
    "fnmadd.q",
    "fadd.q",
    "fsub.q",
    "fmul.q",
    "fdiv.q",
    "fsqrt.q",
    "fsgnj.q",
    "fsgnjn.q",
    "fsgnjx.q",
    "fmin.q",
    "fmax.q",
    "fcvt.s.q",
    "fcvt.q.s",
    "fcvt.d.q",
    "fcvt.q.d",
    "feq.q",
    "flt.q",
    "fle.q",
    "fclass.q",
    "fcvt.w.q",
    "fcvt.wu.q",
    "fcvt.q.w",
    "fcvt.q.wu",
    "fcvt.l.q",
    "fcvt.lu.q",
    "fcvt.q.l",
    "fcvt.q.lu",
    // RV64Zfh
    "flh",
    "fsh",
    "fmadd.h",
    "fmsub.h",
    "fnmsub.h",
    "fnmadd.h",
    "fadd.h",
    "fsub.h",
    "fmul.h",
    "fdiv.h",
    "fsqrt.h",
    "fsgnj.h",
    "fsgnjn.h",
    "fsgnjx.h",
    "fmin.h",
    "fmax.h",
    "fcvt.s.h",
    "fcvt.h.s",
    "fcvt.d.h",
    "fcvt.h.d",
    "fcvt.q.h",
    "fcvt.h.q",
    "feq.h",
    "flt.h",
    "fle.h",
    "fclass.h",
    "fcvt.w.h",
    "fcvt.wu.h",
    "fmv.x.h",
    "fcvt.h.w",
    "fcvt.h.wu",
    "fmv.h.x",
    "fcvt.l.h",
    "fcvt.lu.h",
    "fcvt.h.l",
    "fcvt.h.lu",
    // Zawrs
    "wrs.nto",
    "wrs.sto"

    // Vector?
};



/**
 * 30.10.v Vector Arithmetic Instruction Encoding
 */
typedef enum ENUM_PACK_ATTRIBUTE {
    C_RV64_OPIVV,
    C_RV64_OPFVV,
    C_RV64_OPMVV,
    C_RV64_OPIVI,
    C_RV64_OPIVX,
    C_RV64_OPFVF,
    C_RV64_OPMVX,
    C_RV64_OPCFG
} vector_category_rv64_t;



#endif // INSTRUCTIONS_RV64I_H
