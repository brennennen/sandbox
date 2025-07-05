#ifndef INSTRUCTIONS_RV64_H
#define INSTRUCTIONS_RV64_H

#include <stdint.h>
#include <inttypes.h>

#include "shared/include/binary_utilities.h"

// MARK: Instructions
typedef enum ENUM_PACK_ATTRIBUTE {
    I_RV64I_INVALID,

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
    I_RV64I_FENCE,
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
    I_RV64A_LR_W,
    I_RV64A_SC_W,
    I_RV64A_AMOSWAP_W,
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

    // MARK: RV64F - Floating Point Arithmetic
    I_RV64F_FLW,
    I_RV64F_FSW,
    I_RV64F_FMADD_S,
    I_RV64F_FMSUB_S,
    I_RV64F_FNMSUB_S,
    I_RV64F_FNMADD_S,
    I_RV64F_FADD_S,
    I_RV64F_FSUB_S,
    I_RV64F_FMUL_S,
    I_RV64F_FDIV_S,
    I_RV64F_FSQRT_S,
    I_RV64F_FSGNJ_S,
    I_RV64F_FSGNJN_S,
    I_RV64F_FSGNJX_S,
    I_RV64F_FMIN_S,
    I_RV64F_FMAX_S,
    I_RV64F_FCVT_W_S,
    I_RV64F_FCVT_WU_S,
    I_RV64F_FMV_X_W,
    I_RV64F_FEQ_S,
    I_RV64F_FLT_S,
    I_RV64F_FLE_S,
    I_RV64F_FCLASS_S,
    I_RV64F_FCVT_S_W,
    I_RV64F_FCVT_S_WU,
    I_RV64F_FMV_W_X,
    I_RV64F_FCVT_L_S,
    I_RV64F_FCVT_LU_S,
    I_RV64F_FCVT_S_L,
    I_RV64F_FCVT_S_LU,

    // MARK: RV64D - Double
    // MARK: RV64Q - Quad?
    // MARK: RV64Zfh
    // MARK: Zawrs
} instruction_tag_rv64_t;

/**
 * Lookup table for machine instructions to their associated assembly instruction
 * mnemonics.
 * WARNING: Parallal array with "instruction_tag_rv64_t".
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
    // RV64Q
    // RV64Zfh
    // Zawrs
};

#endif // INSTRUCTIONS_RV64I_H
