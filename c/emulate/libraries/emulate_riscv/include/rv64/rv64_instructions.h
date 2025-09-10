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

    /**
     * MARK: RV64I - Integer
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#rv64
     */
    I_RV64I_LUI, /** lui - Load Upper Immediate - loads an unsigned 20 bit number into the upper 20 bits of a 32 bit int, lower bits being all 0. Then sign extends to 64 bits. */
    I_RV64I_AUIPC, /** auipc - Add Upper Immediate to pc */
    I_RV64I_JAL, /** jal - Jump and Link - Unconditionally jump to symbol and stores the next instruction (after the jal) into a register (usually `ra` abi return address).*/
    I_RV64I_JALR, /** jalr - Jump and Link and Return - Unconditionally jump ... */
    I_RV64I_BEQ, /** beq - Branch/Conditional Jump - Take branch if equal. */
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
     * MARK: RV64Zihintpause
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#zihintpause
     */
    I_RV64I_PAUSE,

    /*
     * MARK: RV64Zifencei
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#zifencei
     */
    I_RV64ZIFENCEI_FENCE_I,

    /*
     * MARK: RV64Zicsr
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#csrinsts
     */
    I_RV64ZICSR_CSRRW, // Control Status Registers atomic Read Write
    I_RV64ZICSR_CSRRS, // atomic Read and Set bits
    I_RV64ZICSR_CSRRC, // atomic Read and Clear bits
    I_RV64ZICSR_CSRRWI, // atomic Read Write Immediate
    I_RV64ZICSR_CSRRSI, // atomic Read and Set bits from Immediate
    I_RV64ZICSR_CSRRCI, // atomic Read and Clear bits from Immediate

    /*
     * MARK: RV64Zicntr
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#counters
     */
    I_RV64ZICNTR_RDCYCLE,
    I_RV64ZICNTR_RDTIME,
    I_RV64ZICNTR_RDINSTRET,

    /*
     * MARK: RV64Zihpm
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#counters
     * hpmcounter3 - hpmcounter31, not sure how to access them though. Maybe Zicntr instructions?
     */

    /*
     * MARK: RV64Zihintntl
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#chap:zihintntl
     * No specific instructions? should each hint be a tag?
     */

    /*
     * MARK: RV64Zihintpause
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#zihintpause
     * No specific instructions? should each hint be a tag?
     */

    /*
     * MARK: RV64Zimop
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#zimop
     * * 32 MOP.R.n instructions (0 - 31, ex: mop.r.0, mop.r.1, ..., mop.r.31)
     * * 8 MOP.RR.n instructions (0 - 7, ex: mop.rr.0, mop.rr.1, ..., mo.rr.7)
     */
    I_RV64ZIMOP_MOP_R_0,
    I_RV64ZIMOP_MOP_R_1,
    I_RV64ZIMOP_MOP_R_2,
    // TODO: 3 - 31
    I_RV64ZIMOP_MOP_RR_0,
    I_RV64ZIMOP_MOP_RR_1,
    I_RV64ZIMOP_MOP_RR_2,
    // TODO: 3 - 7

    /*
     * MARK: RV64Zcmop (depends on "Zcmop")
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#zimop
     * * Eight 16-bit C.MOP.n instructions (n = odds between 1 and 15, ex: c.mop.1, c.mop.3, ..., c.mop.15)
     */
    I_RV64ZCMOP_C_MOP_1,
    I_RV64ZCMOP_C_MOP_3,
    I_RV64ZCMOP_C_MOP_5,
    // TODO: 7, 9, 11, 13, 15

    /*
     * MARK: RV64Zicond
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#Zicond
     */
    I_RV64ZICOND_CZERO_EQZ,
    I_RV64ZICOND_CZERO_NEZ,

    /*
     * MARK: RV64M - Multiplication
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#mstandard
     */
    I_RV64M_MUL, /** Multiply - Multiply 2 64-bit values together. */
    I_RV64M_MULH, /** Multiply High - Multiply 2 64-bit signed values together, interprets the result as 128 bits and returns the upper/high 64 bits. */
    I_RV64M_MULHSU, /** Multiply High Signed by Unsigned - Multiply a 64-bit signed value with a 64-bit unsigned value, interprets the result as 128 bits and returns the upper/high 64 bits. */
    I_RV64M_MULHU, /** Multiply High Unsigned - Multiply 2 64-bit unsigned values together, */
    I_RV64M_DIV,
    I_RV64M_DIVU,
    I_RV64M_REM,
    I_RV64M_REMU,
    I_RV64M_MULW,
    I_RV64M_DIVW,
    I_RV64M_DIVUW,
    I_RV64M_REMW,
    I_RV64M_REMUW,

    /*
     * MARK: Zmmul
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_zmmul_extension_version_1_0
     * Allows for only implementing the multiply and not division/remainder functions for low power embedded devices.
     * No specific instructions added if RV64M is implemented.
     */

    /*
     * MARK: RV64A - Atomic
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#atomics
     */

    /*
     * MARK: RV64Zalrsc
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#sec:lrsc
     */
    I_RV64ZALRSC_LR_W, /** Load-Reserved Word */
    I_RV64ZALRSC_SC_W, /** Store-Conditional Word */
    I_RV64ZALRSC_LR_D,
    I_RV64ZALRSC_SC_D,

    /*
     * MARK: RV64Zaamo
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#sec:amo
     */
    I_RV64ZAAMO_AMOSWAP_W, /** Atomic Swap */
    I_RV64ZAAMO_AMOADD_W,
    I_RV64ZAAMO_AMOXOR_W,
    I_RV64ZAAMO_AMOAND_W,
    I_RV64ZAAMO_AMOOR_W,
    I_RV64ZAAMO_AMOMIN_W,
    I_RV64ZAAMO_AMOMAX_W,
    I_RV64ZAAMO_AMOMINU_W,
    I_RV64ZAAMO_AMOMAXU_W,
    I_RV64ZAAMO_AMOSWAP_D,
    I_RV64ZAAMO_AMOADD_D,
    I_RV64ZAAMO_AMOXOR_D,
    I_RV64ZAAMO_AMOAND_D,
    I_RV64ZAAMO_AMOOR_D,
    I_RV64ZAAMO_AMOMIN_D,
    I_RV64ZAAMO_AMOMAX_D,
    I_RV64ZAAMO_AMOMINU_D,
    I_RV64ZAAMO_AMOMAXU_D,

    /*
     * MARK: Zawrs
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_zawrs_extension_for_wait_on_reservation_set_instructions_version_1_01
     */
    I_RV64ZAWRS_WRS_NTO, /** Stall execution to save power, resume when memory address changes. */
    I_RV64ZAWRS_WRS_STO,/** Stall execution with short upper bound, resume when memory address changes. */

    /*
     * MARK: Zacas
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_zacas_extension_for_atomic_compare_and_swap_cas_instructions_version_1_0_0
     */
    I_RV64ZACAS_AMOCAS_W,
    I_RV64ZACAS_AMOCAS_D,
    I_RV64ZACAS_AMOCAS_Q,

    /*
     * MARK: Zabha
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_zabha_extension_for_byte_and_halfword_atomic_memory_operations_version_1_0
     */
    I_RV64ZABHA_AMOSWAP_B,
    I_RV64ZABHA_AMOADD_B,
    I_RV64ZABHA_AMOAND_B,
    I_RV64ZABHA_AMOOR_B,
    I_RV64ZABHA_AMOXOR_B,
    I_RV64ZABHA_AMOMAX_B,
    I_RV64ZABHA_AMOMAXU_B,
    I_RV64ZABHA_AMOMIN_B,
    I_RV64ZABHA_AMOMINU_B,
    I_RV64ZABHA_AMOCAS_B,
    I_RV64ZABHA_AMOSWAP_H,
    I_RV64ZABHA_AMOADD_H,
    I_RV64ZABHA_AMOAND_H,
    I_RV64ZABHA_AMOOR_H,
    I_RV64ZABHA_AMOXOR_H,
    I_RV64ZABHA_AMOMAX_H,
    I_RV64ZABHA_AMOMAXU_H,
    I_RV64ZABHA_AMOMIN_H,
    I_RV64ZABHA_AMOMINU_H,
    I_RV64ZABHA_AMOCAS_H,

    /*
     * MARK: CMO
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#cmo
     */
    // TODO

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
    I_RV64ZFH_FCVT_Q_H,
    I_RV64ZFH_FCVT_H_Q,
    I_RV64ZFH_FEQ_H,
    I_RV64ZFH_FLT_H,
    I_RV64ZFH_FLE_H,
    I_RV64ZFH_FCLASS_H,
    I_RV64ZFH_FCVT_W_H,
    I_RV64ZFH_FCVT_WU_H,
    I_RV64ZFH_FMV_X_H,
    I_RV64ZFH_FCVT_H_W,
    I_RV64ZFH_FCVT_H_WU,
    I_RV64ZFH_FMV_H_X,
    I_RV64ZFH_FCVT_L_H,
    I_RV64ZFH_FCVT_LU_H,
    I_RV64ZFH_FCVT_H_L,
    I_RV64ZFH_FCVT_H_LU,
    /*
     * TODO: Zfbfmin, Zfbfwma, Zfa, Zfinx, Zdinx, Zhinx, Zhinxmin
     */

    /*
     * MARK: RV64C - Compressed
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#compressed
     */
    // I_RV64C_C_LWSP,
    // I_RV64C_C_LDSP,
    // I_RV64C_C_LQSP,
    // I_RV64C_C_FLWSP,
    // I_RV64C_C_FLDSP,
    // I_RV64C_C_SWSP,
    // I_RV64C_C_SDSP,
    // I_RV64C_C_SQSP,
    // I_RV64C_C_FSWSP,
    // I_RV64C_C_FSDSP,
    // I_RV64C_C_LW,
    // I_RV64C_C_LD,
    // I_RV64C_C_LQ,
    // I_RV64C_C_FLW,
    // I_RV64C_C_FLD,
    // I_RV64C_C_SW,
    // I_RV64C_C_SD,
    // I_RV64C_C_SQ,
    // I_RV64C_C_FSW,
    // I_RV64C_C_FSD,
    // I_RV64C_C_J,
    // I_RV64C_C_JAL,
    // I_RV64C_C_JR,
    // I_RV64C_C_JALR,
    // I_RV64C_C_BEQZ,
    // I_RV64C_C_BNEZ,
    // I_RV64C_C_LI,
    // I_RV64C_C_LUI,
    // I_RV64C_C_ADDI,
    // I_RV64C_C_ADDIW,
    // I_RV64C_C_ADDI16SP,
    // I_RV64C_C_ADDI4SPN,
    // I_RV64C_C_SSLI,
    // I_RV64C_C_SRLI,
    // I_RV64C_C_SRAI,
    // I_RV64C_C_ANDI,
    // I_RV64C_C_MV,
    // I_RV64C_C_ADD,
    // I_RV64C_C_AND,
    // I_RV64C_C_OR,
    // I_RV64C_C_XOR,
    // I_RV64C_C_SUB,
    // I_RV64C_C_ADDW,
    // I_RV64C_C_SUBW,
    // I_RV64C_C_NOP,
    // I_RV64C_C_EBREAK,
    // TODO: chapter 28. "Zc*", @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#Zc

    /*
     * MARK: RV64B - Bit Manipulation
     * Comprised of Zba, Zbb, and Zbs.
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#bits
     */
    // Zba - Address generation - @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#zba
    // Zbb - Basic bit-manipulation - @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#zbb
    // Zbc - Carry-less multiplication - @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#zbc
    // Zbs - Single-bit instructions - @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#zbs
    // Zbkb - Bit-manipulation for Cryptography - @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#zbkb
    // Zbkc - Carry-less multiplication for Cryptography - @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#zbkc
    // Zbkx - Crossbar permutations - @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#zbkx
    // TODO: 29.6 cool optimization guide code snippets related to RV64B that would make for good goalpost tests for this emulator - @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_software_optimization_guide

    /*
     * MARK: RV64V - Vector Operations
     * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#vector
     */
    // TODO: vector load, vector store, vector arithmetic
    // @see 30.6. Configuration-Setting Instructions (vsetvli/vsetivli/vsetvl) (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#sec-vector-config)
    I_RV64V_VSETVLI, /** Set Vector Length and Type Immediate. Sets the csr registers "vtype" and "vl". rs1 is written to vl, and vtype is parsed from the immediate. */
    I_RV64V_VSETIVLI, /** Set Immediate Vector Length and Type Immediate. Sets the csr registers "vtype" and "vl", both from immediate values. */
    I_RV64V_VSETVL, /** Set Vector Length. Sets the csr registers "vtype" and "vl" from register values. vl from rs1 and vtype from rs2. */
    // @see 30.7.4 Vector Unit-Stride Instructions (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_vector_unit_stride_instructions)
    I_RV64V_VLE8_V,
    I_RV64V_VLE16_V,
    I_RV64V_VLE32_V,
    I_RV64V_VLE64_V,
    I_RV64V_VSE8_V,
    I_RV64V_VSE16_V,
    I_RV64V_VSE32_V,
    I_RV64V_VSE64_V,
    I_RV64V_VLM_V,
    I_RV64V_VSM_V,
    // @see 30.7.5 Vector Constant-Stride Instructions (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_vector_constant_stride_instructions)
    I_RV64V_VLSE8_V,
    I_RV64V_VLSE16_V,
    I_RV64V_VLSE32_V,
    I_RV64V_VLSE64_V,
    I_RV64V_VSSE8_V,
    I_RV64V_VSSE16_V,
    I_RV64V_VSSE32_V,
    I_RV64V_VSSE64_V,
    // @see 30.7.6 Vector Indexed Instructions (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_vector_indexed_instructions)
    I_RV64V_VLUXEI8_V, /** Vector index-unordered load instruction */
    I_RV64V_VLUXEI16_V,
    I_RV64V_VLUXEI32_V,
    I_RV64V_VLUXEI64_V,
    I_RV64V_VLOXEI8_V, /** Vector indexed-ordered load instruction */
    I_RV64V_VLOXEI16_V,
    I_RV64V_VLOXEI32_V,
    I_RV64V_VLOXEI64_V,
    I_RV64V_VSUXEI8_V, /** Vector indexed-unordered store instruction */
    I_RV64V_VSUXEI16_V,
    I_RV64V_VSUXEI32_V,
    I_RV64V_VSUXEI64_V,
    I_RV64V_VSOXEI8_V, /** Vector indexed-ordered store instruction */
    I_RV64V_VSOXEI16_V,
    I_RV64V_VSOXEI32_V,
    I_RV64V_VSOXEI64_V,
    // @see 30.7.7 Unit-stride Fault-Only-First Loads (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_unit_stride_fault_only_first_loads)
    I_RV64V_VLE8FF_V, /** Vector unit-stride fault-only-first loads */
    I_RV64V_VLE16FF_V,
    I_RV64V_VLE32FF_V,
    I_RV64V_VLE64FF_V,
    // @see 30.7.8.1 Vector Unit-Stride Segment Loads and Stores
    // TODO: vlseg<nf>e<eew>.v, vsseg<nf>e<eew>.v
    // @see 30.7.8.2 Vector Constant-Stride Segment Loads and Stores
    // TODO: vlsseg<nf>e<eew>.v, vssseg<nf>e<eew>.v
    // @see 30.7.8.3 Vector Indexed Segment Loads and Stores
    // TODO: vluxseg<nf>ei<eew>.v, vloxseg<nf>ei<eew>.v, vsuxseg<nf>ei<eew>.v, vsoxseg<nf>ei<eew>.v
    // @see 30.7.9 Vector Load/Store Whole Register Instructions (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_vector_loadstore_whole_register_instructions)
    // TODO: VL*R*, VS*R*
    // @see 30.11.1 Vector Single-Width Integer Add and Subtract (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_vector_single_width_integer_add_and_subtract)
    I_RV64V_VADD_IVV,
    I_RV64V_VADD_IVX,
    I_RV64V_VADD_IVI,
    I_RV64V_VSUB_IVV,
    I_RV64V_VSUB_IVX,
    I_RV64V_VRSUB_IVX,
    I_RV64V_VRSUB_IVI,
    // 30.11.2 Vector Widening Integer Add/Subtract
    // todo - next
    // @see 30.11.9 Vector Integer Min/Max Instructions (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_vector_integer_minmax_instructions)
    //I_RV64_VMINU_VV,
    //I_RV64_VMIN_VX,
    // ...
    // 30.11.10
    // ...

    I_RV64V_VOPVV, /** Assembly syntax pattern for vector binary arithmetic instructions, integer vector-vector */
    I_RV64V_VOPVX, /** integer vector-scalar */
    I_RV64V_VOPVI, /** integer vector-immediate */
    I_RV64V_VFOP_VV, /** float vector-vector */
    I_RV64V_VFOP_VF, /** float vector-scalar */
    I_RV64V_VADC_VMM, /** vector addition with carry */
    // @see 30.10.2 Widening Vector Arithmetic Instructions (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#sec-widening)
    I_RV64V_VWOP_VV, /** in */

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
    // RV64Zihintpause
    "pause",
    // RV64Zifencei
    "fence.i",
    // RV64Zicsr
    "csrrw",
    "csrrs",
    "csrrc",
    "csrrwi",
    "csrrsi",
    "csrrci",
    // RV64Zicntr
    "rdcycle",
    "rdtime",
    "rdinstret",
    // RV64Zihpm
    // RV64Zihintntl
    // RV64Zihintpause
    // RV64Zimop
    "mop.r.0",
    "mop.r.1",
    "mop.r.2",
    "mop.rr.0",
    "mop.rr.1",
    "mop.rr.2",
    // RV64Zcmop
    "c.mop.1",
    "c.mop.3",
    "c.mop.5",
    // RV64Zicond
    "czero.eqz",
    "czero.nez",
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
    // RV64Zalrsc
    "lr.w",
    "sc.w",
    "lr.d",
    "sc.d",
    // RV64Zaamo
    "amoswap.w",
    "amoadd.w",
    "amoxor.w",
    "amoand.w",
    "amoor.w",
    "amomin.w",
    "amomax.w",
    "amominu.w",
    "amomaxu.w",
    "amoswap.d",
    "amoadd.d",
    "amoxor.d",
    "amoand.d",
    "amoor.d",
    "amomin.d",
    "amomax.d",
    "amominu.d",
    "amomaxu.d",
    // RV64ZAAMO
    "wrs.nto",
    "wrs.sto",
    // RV64Zacas
    "amo.cas.w",
    "amo.cas.d",
    "amo.cas.q",
    // RV64Zabha
    "amoswap.b",
    "amoadd.b",
    "amoand.b",
    "amoor.b",
    "amoxor.b",
    "amomax.b",
    "amomaxu.b",
    "amomin.b",
    "amominu.b",
    "amocas.b",
    "amoswap.h",
    "amoadd.h",
    "amoand.h",
    "amoor.h",
    "amoxor.h",
    "amomax.h",
    "amomaxu.h",
    "amomin.h",
    "amominu.h",
    "amocas.h",
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
    // RV64V
    "vsetvli",
    "vsetivli",
    "vsetvl",
    "vle8.v",
    "vle16.v",
    "vle32.v",
    "vle64.v",
    "vse8.v",
    "vse16.v",
    "vse32.v",
    "vse64.v",
    "vlm.v",
    "vsm.v",
    "vlse8.v",
    "vlse16.v",
    "vlse32.v",
    "vlse64.v",
    "vsse8.v",
    "vsse16.v",
    "vsse32.v",
    "vsse64.v",
    "vluxei8.v",
    "vluxei16.v",
    "vluxei32.v",
    "vluxei64.v",
    "vloxei8.v",
    "vloxei16.v",
    "vloxei32.v",
    "vloxei64.v",
    "vsuxei8.v",
    "vsuxei16.v",
    "vsuxei32.v",
    "vsuxei64.v",
    "vsoxei8.v",
    "vsoxei16.v",
    "vsoxei32.v",
    "vsoxei64.v",
    "vle8ff.v",
    "vle16ff.v",
    "vle32ff.v",
    "vle64ff.v",
    // @see 30.11.1 Vector Single-Width Integer Add and Subtract (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_vector_single_width_integer_add_and_subtract)
    "vadd.vv",
    "vadd.vx",
    "vadd.vi",
    "vsub.vv",
    "vsub.vx",
    "vrsub.vx",
    "vrsub.vi",
    // todo: 30.11.2

    "vop.vv",
    "vop.vx",
    "vop.vi",
    "vfop.vv",
    "vfop.vf",
    // I_RV64V_VOPVV, /** Assembly syntax pattern for vector binary arithmetic instructions, integer vector-vector */
    // I_RV64V_VOPVX, /** integer vector-scalar */
    // I_RV64V_VOPVI, /** integer vector-immediate */
    // I_RV64V_VFOP_VV, /** float vector-vector */
    // I_RV64V_VFOP_VF, /** float vector-scalar */
    // I_RV64V_VADC_VMM, /** vector addition with carry */
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
