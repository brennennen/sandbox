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
    I_RV64M_MUL,
    I_RV64M_MULH,
    I_RV64M_MULHSU,
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
};

#endif // INSTRUCTIONS_RV64I_H
