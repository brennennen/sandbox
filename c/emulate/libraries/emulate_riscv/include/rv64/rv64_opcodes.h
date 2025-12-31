/**
 * Opcodes and any additional data used as secondary opcodes to identify
 * instructions. funct3 is commonly used to store a second order opcodes.
 * 
 * Naming convention:
 * * For the simplest case, where just the opcode is used: "OP_{operation name}"
 * * When funct3 is used as a secondary opcode: "OPGRP_{group name}__F3_{operation name}"
 * * When funct3 and funct7 is used as a secondary opcode: 
 * "OPGRP_{group name}__F3GRP_{funct3 group}__F7_{operation name}"
 * * In some places, FUNCT2 is used or moved around, but the pattern remains
 * 
 * Shorthands:
 * * "OP" - Op Code - Operation Code
 * * "OPGRP" - When the opcode is shared between many instructions, and another field is
 * used to denote the exact operation, the "op" field is an "op code group".
 * * "F3" - The "funct3" field that is commonly used as a secondary opcode field (see spec).
 * * "F7" - The "funct7" field that is commonly used as a tertiary opcode field (see spec).
 */

#ifndef RV64_OPCODES
#define RV64_OPCODES

#include <stdint.h>

// MARK: RV64I
constexpr uint8_t OP_LUI = 0b0110111;
constexpr uint8_t OP_AUIPC = 0b0010111;
constexpr uint8_t OP_JAL = 0b1101111;
constexpr uint8_t OP_JALR = 0b1100111;

constexpr uint8_t OPGRP_BRANCH =                    0b1100011;
constexpr uint8_t OPGRP_BRANCH__F3_BEQ =            0b000;
constexpr uint8_t OPGRP_BRANCH__F3_BNE =            0b001;
constexpr uint8_t OPGRP_BRANCH__F3_BLT =            0b100;
constexpr uint8_t OPGRP_BRANCH__F3_BGE =            0b101;
constexpr uint8_t OPGRP_BRANCH__F3_BLTU =           0b110;
constexpr uint8_t OPGRP_BRANCH__F3_BGEU =           0b111;

constexpr uint8_t OPGRP_LOAD =                      0b0000011;
constexpr uint8_t OPGRP_LOAD__F3_LB =               0b000;
constexpr uint8_t OPGRP_LOAD__F3_LH =               0b001;
constexpr uint8_t OPGRP_LOAD__F3_LW =               0b010;
constexpr uint8_t OPGRP_LOAD__F3_LBU =              0b100;
constexpr uint8_t OPGRP_LOAD__F3_LHU =              0b101;
constexpr uint8_t OPGRP_LOAD__F3_LWU =              0b110;
constexpr uint8_t OPGRP_LOAD__F3_LD =               0b011;

constexpr uint8_t OPGRP_STORE =                     0b0100011;
constexpr uint8_t OPGRP_STORE__F3_SB =              0b000;
constexpr uint8_t OPGRP_STORE__F3_SH =              0b001;
constexpr uint8_t OPGRP_STORE__F3_SW =              0b010;
constexpr uint8_t OPGRP_STORE__F3_SD =              0b011;

constexpr uint8_t OPGRP_ALU_IMMED =                   0b0010011;
constexpr uint8_t OPGRP_ALU_IMMED__F3_ADDI =          0b000;
constexpr uint8_t OPGRP_ALU_IMMED__F3_SLTI =          0b010;
constexpr uint8_t OPGRP_ALU_IMMED__F3_SLTIU =         0b011;
constexpr uint8_t OPGRP_ALU_IMMED__F3_XORI =          0b100;
constexpr uint8_t OPGRP_ALU_IMMED__F3_ORI =           0b110;
constexpr uint8_t OPGRP_ALU_IMMED__F3_ANDI =          0b111;
constexpr uint8_t OPGRP_ALU_IMMED__F3_SLLI =          0b001;
constexpr uint8_t OPGRP_ALU_IMMED__F3GRP_SHIFT_RIGHT =            0b101;
constexpr uint8_t OPGRP_ALU_IMMED__F3GRP_SHIFT_RIGHT__F7_SRLI =   0b0000000;
constexpr uint8_t OPGRP_ALU_IMMED__F3GRP_SHIFT_RIGHT__F7_SRAI =   0b0100000;

constexpr uint8_t OPGRP_MATH_REG =                              0b0110011;
constexpr uint8_t OPGRP_MATH_REG__F3GRP_MATH =               0b000;
constexpr uint8_t OPGRP_MATH_REG__F3GRP_MATH__F7_ADD =       0b0000000;
constexpr uint8_t OPGRP_MATH_REG__F3GRP_MATH__F7_SUB =       0b0100000;
constexpr uint8_t OPGRP_MATH_REG__F3GRP_MATH__F7_MUL =       0b0000001;
// ...
constexpr uint8_t OPGRP_MATH_REG__F3GRP100 = 0b100;
constexpr uint8_t OPGRP_MATH_REG__F3GRP100__F7_XOR = 0b0000000;
constexpr uint8_t OPGRP_MATH_REG__F3GRP100__F7_DIV = 0b0000001;
// ...
constexpr uint8_t OPGRP_MATH_REG__F3GRP110 = 0b110;
constexpr uint8_t OPGRP_MATH_REG__F3GRP110__F7_OR = 0b0000000;
constexpr uint8_t OPGRP_MATH_REG__F3GRP110__F7_REM = 0b0000001;
// ...
constexpr uint8_t OPGRP_MATH_REG__F3GRP111 = 0b111;
constexpr uint8_t OPGRP_MATH_REG__F3GRP111__F7_AND = 0b0000000;
constexpr uint8_t OPGRP_MATH_REG__F3GRP111__F7_REMU = 0b0000001;

// ...

constexpr uint8_t OPGRP_IMMED_WIDE =                0b0011011;
constexpr uint8_t OPGRP_IMMED_WIDE__F3_ADDIW =      0b000;
constexpr uint8_t OPGRP_IMMED_WIDE__F3_SLLIW =      0b001;
constexpr uint8_t OPGRP_IMMED_WIDE__F3GRP_SHIFT_RIGHT = 0b101;
constexpr uint8_t OPGRP_IMMED_WIDE__F3GRP_SHIFT_RIGHT__F7_SRLIW = 0b0000000;
constexpr uint8_t OPGRP_IMMED_WIDE__F3GRP_SHIFT_RIGHT__F7_SRAIW = 0b0100000;

// ...
constexpr uint8_t OPGRP_REG_WIDE =                  0b0111011;
constexpr uint8_t OPGRP_REG_WIDE__F3GRP000 = 0b000;
constexpr uint8_t OPGRP_REG_WIDE__F3GRP000__F7_ADDW = 0b0000000;
constexpr uint8_t OPGRP_REG_WIDE__F3GRP000__F7_MULW = 0b0000001;
constexpr uint8_t OPGRP_REG_WIDE__F3GRP000__F7_SUBW = 0b0100000;

// ...

// MARK: RV64F/D/Q/H
#define RV64_OPCODE_GROUP_FLOAT_LOAD        0b0000111
#define RV64_OPCODE_GROUP_FLOAT_LOAD_FUNCT3_FLW     0b010
#define RV64_OPCODE_GROUP_FLOAT_LOAD_FUNCT3_FLD     0b011
#define RV64_OPCODE_GROUP_FLOAT_LOAD_FUNCT3_FLQ     0b100
#define RV64_OPCODE_GROUP_FLOAT_LOAD_FUNCT3_FLH     0b001
// ...

#define RV64_OPCODE_GROUP_FLOAT_STORE       0b0100111
#define RV64_OPCODE_GROUP_FLOAT_STORE_FUNCT3_FSW     0b010
// ...

#define RV64_OPCODE_GROUP_FLOAT_FMADD       0b1000011
#define RV64_OPCODE_GROUP_FLOAT_FMADD_FUNCT2_S      0b00
#define RV64_OPCODE_GROUP_FLOAT_FMADD_FUNCT2_D      0b01
#define RV64_OPCODE_GROUP_FLOAT_FMADD_FUNCT2_Q      0b11
#define RV64_OPCODE_GROUP_FLOAT_FMADD_FUNCT2_H      0b10

#define RV64_OPCODE_GROUP_FLOAT_FMSUB       0b1000111
#define RV64_OPCODE_GROUP_FLOAT_FMSUB_FUNCT2_S      0b00
#define RV64_OPCODE_GROUP_FLOAT_FMSUB_FUNCT2_D      0b01
#define RV64_OPCODE_GROUP_FLOAT_FMSUB_FUNCT2_Q      0b11
#define RV64_OPCODE_GROUP_FLOAT_FMSUB_FUNCT2_H      0b10


// MARK: RV64M

// MARK: RV64A

// MARK: RV64V

#endif // RV64_OPCODES
