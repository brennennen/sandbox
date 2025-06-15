#ifndef INSTRUCTIONS_RV64I_H
#define INSTRUCTIONS_RV64I_H

#include <stdint.h>
#include <inttypes.h>

#include "shared/include/binary_utilities.h"

// MARK: Instructions
typedef enum ENUM_PACK_ATTRIBUTE {
    // RV32I Base Instructions
    I_RV64I_INVALID,
    I_RV64I_LUI,
    I_RV64I_AUIPC,
    I_RV64I_JAL,
    I_RV64I_JALR,
    I_RV64I_BEQ,
    I_RV64I_BNE,
    I_RV64I_BLT,
    I_RV64I_BGE,
    I_RV64I_BLTU,
    I_RV64I_BGEU,
    I_RV64I_LB,
    I_RV64I_LH,
    /** Load Word? */
    I_RV64I_LW,
    I_RV64I_LBU,
    I_RV64I_LHU,
    I_RV64I_SB,
    I_RV64I_SH,
    /** Store Word? */
    I_RV64I_SW,
    /** Add Immediate */
    I_RV64I_ADDI,
    /** Set Less Than Immediate */
    I_RV64I_SLTI,
    /** Set Less Than Immediate Unsigned */
    I_RV64I_SLTIU,
    /** XOR Immediate */
    I_RV64I_XORI,
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

    // RV64I Extra Instructions
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
} instruction_tag_rv64i_t;

/**
 * Lookup table for machine instructions to their associated assembly instruction
 * mnemonics.
 * WARNING: Parallal array with "instruction_tag_rv64i_t".
 */
static char rv64i_instruction_tag_mnemonic[][16] = {
    "invalid",
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
};

#endif // INSTRUCTIONS_RV64I_H
