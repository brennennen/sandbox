/**
 * RV64C module takes 16-bit compressed expressions and expands them
 * into standard/full 32-bit instructions.
 */

#include <stdint.h>

#include "rv64/rv64_opcodes.h"
#include "rv64/rv64_registers.h"

const uint32_t RV64_ILLEGAL_INSTRUCTION = ((uint32_t)0);

/**
 * Extracts a register from a compressed instruction.
 * "Prime" registers are 3 bits that only cover some of the middle
 * registers (denoted by rs1', rs2', rd' in spec).
 * @param instruction Instruction to extract the register from.
 * @param start_pos Start bit of the register to extract.
 */
static uint8_t rv64c_decode_prime_register(uint16_t instruction, uint8_t start_pos) {
    // Compressed instruction registers use registers 8 - 15
    return 8 + ((instruction >> start_pos) & 0b111);
}

// Helper for Loads (I-Type) and Arith-Immediate
static uint32_t build_i_type(
    uint32_t opcode,
    uint32_t funct3,
    uint32_t rd,
    uint32_t rs1,
    uint32_t imm
) {
    return (imm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

// Helper for Stores (S-Type)
static uint32_t build_s_type(
    uint32_t opcode,
    uint32_t funct3,
    uint32_t rs1,
    uint32_t rs2,
    uint32_t imm
) {
    uint32_t imm_low = imm & 0x1F;
    uint32_t imm_high = (imm >> 5) & 0x7F;
    return (imm_high << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (imm_low << 7) | opcode;
}

// MARK: Q0 - Load and Store
constexpr uint8_t OPGRP_CQ0 = 0b00;

// #define CI_RV64_CADDI4SPN   0b000
constexpr uint8_t OPGRP_CQ0__F3_FLD = 0b001;
constexpr uint8_t OPGRP_CQ0__F3_LW = 0b010;
// constexpr uint8_t OPGRP_CQ0__F3_LW = 0b010; - RV32 only
constexpr uint8_t OPGRP_CQ0__F3_LD = 0b011;  // RV64 only
constexpr uint8_t OPGRP_CQ0__F3_FSD = 0b101;
constexpr uint8_t OPGRP_CQ0__F3_SW = 0b110;
// constexpr uint8_t OPGRP_CQ0__F3_FSW = 0b111; - RV32 only
constexpr uint8_t OPGRP_CQ0__F3_SD = 0b111;  // RV64 only

/**
 * Expands C.FLD, C.LD, C.FSD, and C.SD compressed instructions into their
 * full versions.
 * See 28.3.2 Register-Based Loads and Stores
 */
static uint32_t expand_load_store_fld_ld_fsd_sd(
    uint16_t instruction,
    uint8_t opcode,
    uint8_t funct3
) {
    uint32_t imm = ((instruction >> 10) & 0b111) << 3   // Bits 5:3
                   | ((instruction >> 5) & 0b11) << 6;  // Bits 7:6
    uint32_t rs1 = rv64c_decode_prime_register(instruction, 7);
    uint32_t rd = rv64c_decode_prime_register(instruction, 2);
    return (imm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

static uint32_t expand_q0_fld(uint16_t instruction, uint8_t opcode, uint8_t funct3) {
    uint32_t imm = ((instruction >> 10) & 0b111) << 3   // Bits 5:3
                   | ((instruction >> 5) & 0b11) << 6;  // Bits 7:6
    uint32_t rs1 = rv64c_decode_prime_register(instruction, 7);
    uint32_t rd = rv64c_decode_prime_register(instruction, 2);
    return (imm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

static uint32_t expand_q0_ld(uint16_t instruction, uint8_t opcode, uint8_t funct3) {
    uint32_t imm = ((instruction >> 10) & 0b111) << 3   // Bits 5:3
                   | ((instruction >> 5) & 0b11) << 6;  // Bits 7:6
    uint32_t rs1 = rv64c_decode_prime_register(instruction, 7);
    uint32_t rd = rv64c_decode_prime_register(instruction, 2);
    // TOOD: call "build_x_type"
    return (imm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

/**
 * Parses and expands C.LW and C.SW compressed instructions into their
 * full versions.
 * See 28.3.2 Register-Based Loads and Stores
 */
static uint32_t expand_q0_lw(uint16_t instruction) {
    uint32_t imm = ((instruction >> 6) & 0b1) << 2       // Bit 2
                   | ((instruction >> 10) & 0b111) << 3  // Bits 5:3
                   | ((instruction >> 5) & 0b1) << 6;    // Bit 6
    uint32_t rs1 = rv64c_decode_prime_register(instruction, 7);
    uint32_t rd = rv64c_decode_prime_register(instruction, 2);
    return (imm << 20) | (rs1 << 15) | (OPGRP_LOAD__F3_LW << 12) | (rd << 7) | OPGRP_LOAD;
}

static uint32_t expand_q0_sw(uint16_t instruction) {
    uint32_t imm = ((instruction >> 6) & 0b1) << 2       // Bit 2
                   | ((instruction >> 10) & 0b111) << 3  // Bits 5:3
                   | ((instruction >> 5) & 0b1) << 6;    // Bit 6
    uint32_t rs1 = rv64c_decode_prime_register(instruction, 7);
    uint32_t rs2 = rv64c_decode_prime_register(instruction, 2);
    return build_s_type(OPGRP_STORE, OPGRP_STORE__F3_SW, rs1, rs2, imm);
}

// OPGRP_STORE, OPGRP_STORE__F3_SD
static uint32_t expand_q0_sd(uint16_t instruction) {
    uint32_t imm = ((instruction >> 10) & 0b111) << 3 | ((instruction >> 5) & 0b11) << 6;
    uint32_t rs1 = rv64c_decode_prime_register(instruction, 7);
    uint32_t rs2 = rv64c_decode_prime_register(instruction, 2);
    return build_s_type(OPGRP_STORE, OPGRP_STORE__F3_SD, rs1, rs2, imm);
}

// MARK: Q1 - ALU and Branch
constexpr uint8_t OPGRP_CQ1 = 0b01;
constexpr uint8_t OPGRP_CQ1__F3_ADDI = 0b000;
constexpr uint8_t OPGRP_CQ1__F3_ADDIW = 0b001;
constexpr uint8_t OPGRP_CQ1__F3_LI = 0b010;
constexpr uint8_t OPGRP_CQ1__F3GRP_ADDI16SP_LUI = 0b011;
constexpr uint8_t OPGRP_CQ1__F3GRP_ALU = 0b100;
constexpr uint8_t OPGRP_CQ1__F3GRP_ALU__F2_SRLI = 0b00;
constexpr uint8_t OPGRP_CQ1__F3GRP_ALU__F2_SRAI = 0b01;
constexpr uint8_t OPGRP_CQ1__F3GRP_ALU__F2_ANDI = 0b10;
constexpr uint8_t OPGRP_CQ1__F3GRP_ALU__F2GRP_LOGICAL = 0b11;
constexpr uint8_t OPGRP_CQ1__F3GRP_ALU__F2GRP_LOGICAL__F1GRP_WORD = 0b0;
constexpr uint8_t OPGRP_CQ1__F3GRP_ALU__F2GRP_LOGICAL__F1GRP_WORD__F2_SUB = 0b00;
constexpr uint8_t OPGRP_CQ1__F3GRP_ALU__F2GRP_LOGICAL__F1GRP_WORD__F2_XOR = 0b01;
constexpr uint8_t OPGRP_CQ1__F3GRP_ALU__F2GRP_LOGICAL__F1GRP_WORD__F2_OR = 0b10;
constexpr uint8_t OPGRP_CQ1__F3GRP_ALU__F2GRP_LOGICAL__F1GRP_WORD__F2_AND = 0b11;
constexpr uint8_t OPGRP_CQ1__F3GRP_ALU__F2GRP_LOGICAL__F1GRP_WIDE = 0b1;
constexpr uint8_t OPGRP_CQ1__F3GRP_ALU__F2GRP_LOGICAL__F1GRP_WIDE__F2_SUBW = 0b00;
constexpr uint8_t OPGRP_CQ1__F3GRP_ALU__F2GRP_LOGICAL__F1GRP_WIDE__F2_ADDW = 0b01;
constexpr uint8_t OPGRP_CQ1__F3_J = 0b101;
constexpr uint8_t OPGRP_CQ1__F3_BEQZ = 0b110;
constexpr uint8_t OPGRP_CQ1__F3_BNEZ = 0b111;

/**
 * See 27.5.2 Integer Register-Immediate Operations
 * @see
 * https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_integer_register_immediate_operations
 */
static uint32_t expand_q1_addi_addiw_li(uint16_t instruction, uint8_t opcode, uint8_t funct3) {
    int32_t imm = ((instruction >> 2) & 0b11111) | (((instruction >> 12) & 0b1) << 5);
    if (imm & 0x20) {
        imm |= 0xFFFFFFC0;  // Sign extend 6-bit immediate
    }
    uint32_t rd = (((instruction) >> 7) & 0b11111);
    if (rd == 0) {
        return RV64_ILLEGAL_INSTRUCTION;
    }
    return ((imm) << 20) | ((rd) << 15) | ((funct3) << 12) | ((rd) << 7) | (opcode);
}

static uint32_t expand_q1_li(uint16_t instruction) {
    uint32_t rd = (instruction >> 7) & 0b11111;
    int32_t imm6 = ((instruction >> 12) & 0b1) << 5 | ((instruction >> 2) & 0b11111);
    if (imm6 & 0x20) {
        imm6 |= 0xFFFFFFC0;
    }
    uint32_t imm12 = (uint32_t)imm6 & 0xFFF;
    return (imm12 << 20) | (0 << 15) | (0x0 << 12) | (rd << 7) | 0x13;
}

static uint32_t expand_q1_addi16sp_lui(uint16_t instruction) {
    uint32_t rd = (instruction >> 7) & 0b11111;
    if (rd == 0) {
        return RV64_ILLEGAL_INSTRUCTION;
    }
    if (rd == 2) {
        // TODO: addi16sp
        return 0;
    } else {
        int32_t imm6 = ((instruction >> 12) & 0x1) << 5 | ((instruction >> 2) & 0x1F);
        if (imm6 & 0x20) {
            imm6 |= 0xFFFFFFC0;
        }
        uint32_t imm20 = (uint32_t)imm6 & 0xFFFFF;
        return (imm20 << 12) | (rd << 7) | 0x37;
    }
}

static uint32_t expand_q1_srli(uint16_t instruction) {
    uint32_t rd_rs1 = rv64c_decode_prime_register(instruction, 7);
    uint32_t shamt = ((instruction >> 12) & 0x1) << 5 | ((instruction >> 2) & 0b11111);
    return (OPGRP_ALU_IMMED__F3GRP_SHIFT_RIGHT__F7_SRLI << 25) | (shamt << 20) | (rd_rs1 << 15)
           | (OPGRP_ALU_IMMED__F3GRP_SHIFT_RIGHT << 12) | (rd_rs1 << 7) | OPGRP_ALU_IMMED;
}

static uint32_t expand_q1_srai(uint16_t instruction) {
    uint32_t rd_rs1 = rv64c_decode_prime_register(instruction, 7);
    uint32_t shamt = ((instruction >> 12) & 0x1) << 5 | ((instruction >> 2) & 0b11111);
    return (OPGRP_ALU_IMMED__F3GRP_SHIFT_RIGHT__F7_SRAI << 25) | (shamt << 20) | (rd_rs1 << 15)
           | (OPGRP_ALU_IMMED__F3GRP_SHIFT_RIGHT << 12) | (rd_rs1 << 7) | OPGRP_ALU_IMMED;
}

static uint32_t expand_q1_andi(uint16_t instruction) {
    uint32_t rd_rs1 = rv64c_decode_prime_register(instruction, 7);
    int32_t imm6 = ((instruction >> 12) & 0x1) << 5 | ((instruction >> 2) & 0b11111);
    if (imm6 & 0x20) {  // sign extend
        imm6 |= 0xFFFFFFC0;
    }
    uint32_t imm12 = (uint32_t)imm6 & 0xFFF;  // mask to 12 bits for expanded instruction
    return (imm12 << 20) | (rd_rs1 << 15) | (OPGRP_ALU_IMMED__F3_ANDI << 12) | (rd_rs1 << 7)
           | OPGRP_ALU_IMMED;
}

static uint32_t expand_q1_sub(uint16_t instruction) {
    uint32_t rd_rs1 = rv64c_decode_prime_register(instruction, 7);
    uint32_t rs2 = rv64c_decode_prime_register(instruction, 2);
    return (OPGRP_MATH_REG__F3GRP_MATH__F7_SUB << 25) | (rs2 << 20) | (rd_rs1 << 15)
           | (OPGRP_MATH_REG__F3GRP_MATH << 12) | (rd_rs1 << 7) | OPGRP_MATH_REG;
}

static uint32_t expand_q1_xor(uint16_t instruction) {
    uint32_t rd_rs1 = rv64c_decode_prime_register(instruction, 7);
    uint32_t rs2 = rv64c_decode_prime_register(instruction, 2);
    return (OPGRP_MATH_REG__F3GRP100__F7_XOR << 25) | (rs2 << 20) | (rd_rs1 << 15)
           | (OPGRP_MATH_REG__F3GRP100 << 12) | (rd_rs1 << 7) | OPGRP_MATH_REG;
}

static uint32_t expand_q1_or(uint16_t instruction) {
    uint32_t rd_rs1 = rv64c_decode_prime_register(instruction, 7);
    uint32_t rs2 = rv64c_decode_prime_register(instruction, 2);
    return (OPGRP_MATH_REG__F3GRP110__F7_OR << 25) | (rs2 << 20) | (rd_rs1 << 15)
           | (OPGRP_MATH_REG__F3GRP110 << 12) | (rd_rs1 << 7) | OPGRP_MATH_REG;
}

static uint32_t expand_q1_and(uint16_t instruction) {
    uint32_t rd_rs1 = rv64c_decode_prime_register(instruction, 7);
    uint32_t rs2 = rv64c_decode_prime_register(instruction, 2);
    return (OPGRP_MATH_REG__F3GRP111__F7_AND << 25) | (rs2 << 20) | (rd_rs1 << 15)
           | (OPGRP_MATH_REG__F3GRP111 << 12) | (rd_rs1 << 7) | OPGRP_MATH_REG;
}

static uint32_t expand_q1_subw(uint16_t instruction) {
    uint32_t rd_rs1 = rv64c_decode_prime_register(instruction, 7);
    uint32_t rs2 = rv64c_decode_prime_register(instruction, 2);
    return (OPGRP_REG_WIDE__F3GRP000__F7_SUBW << 25) | (rs2 << 20) | (rd_rs1 << 15)
           | (OPGRP_REG_WIDE__F3GRP000 << 12) | (rd_rs1 << 7) | OPGRP_REG_WIDE;
}

static uint32_t expand_q1_addw(uint16_t instruction) {
    uint32_t rd_rs1 = rv64c_decode_prime_register(instruction, 7);
    uint32_t rs2 = rv64c_decode_prime_register(instruction, 2);
    return (OPGRP_REG_WIDE__F3GRP000__F7_ADDW << 25) | (rs2 << 20) | (rd_rs1 << 15)
           | (OPGRP_REG_WIDE__F3GRP000 << 12) | (rd_rs1 << 7) | OPGRP_REG_WIDE;
}

static uint32_t expand_q1_j(uint16_t instruction) {
    return 0;
}

static uint32_t expand_q1_beqz(uint16_t instruction) {
    return 0;
}

static uint32_t expand_q1_bnez(uint16_t instruction) {
    return 0;
}

static uint32_t expand_q1_srli_srai(
    uint16_t instruction,
    uint8_t opcode,
    uint8_t funct3,
    uint8_t funct7
) {
    return 0;
}

static uint32_t expand_jump_j(uint16_t instruction, uint8_t opcode, uint8_t funct3) {
    return 0;
}

// MARK: Q2 - STACK and Misc
constexpr uint8_t OPGRP_CQ2 = 0b10;
constexpr uint8_t OPGRP_CQ2__F3_SLLI = 0b000;
constexpr uint8_t OPGRP_CQ2__F3_FLDSP = 0b001;
constexpr uint8_t OPGRP_CQ2__F3_LWSP = 0b010;
// constexpr uint8_t OPGRP_CQ2__F3_FLWSP =         0b011; - RV32 only
constexpr uint8_t OPGRP_CQ2__F3_LDSP = 0b011;  // RV64 only
constexpr uint8_t OPGRP_CQ2__F3GRP_JUMP = 0b100;
constexpr uint8_t OPGRP_CQ2__F3GRP_JUMP__F1GRP0 = 0b0;  // MV and JR
constexpr uint8_t OPGRP_CQ2__F3GRP_JUMP__F1GRP1 = 0b1;  // EBREAK, JALR, and ADD

// ...
// TODO: JR, MV, EBREAK, JALR, ADD
// ...
constexpr uint8_t OPGRP_CQ2__F3_FSDSP = 0b101;
constexpr uint8_t OPGRP_CQ2__F3_SWSP = 0b110;
// constexpr uint8_t OPGRP_CQ2__F3_FSWSP =         0b111; - RV32 only
constexpr uint8_t OPGRP_CQ2__F3_SDSP = 0b111;  // RV64 only

// #define CI_RV64C_FLWSP      0b011 - RV32 only
#define CI_RV64C_LDSP   0b011  // RV64 only
#define CI_RV64C_JR     0b100
#define CI_RV64C_MV     0b100
#define CI_RV64C_EBREAK 0b100
#define CI_RV64C_JALR   0b100
#define CI_RV64C_ADD    0b100
#define CI_RV64C_FSDSP  0b101
#define CI_RV64C_SWSP   0b110
// #define CI_RV64C_FSWSP      0b111 - RV32 only
#define CI_RV64C_SDSP 0b111  // RV64 only

static uint32_t expand_q2_slli(uint16_t instruction) {
    int32_t nzuimm = ((instruction >> 2) & 0b11111);  // non-zero unsigned immediate
    uint32_t rs1_rd = (((instruction) >> 7) & 0b11111);
    if (rs1_rd == 0 || nzuimm == 0) {
        return RV64_ILLEGAL_INSTRUCTION;
    }
    return ((nzuimm) << 20) | ((rs1_rd) << 15) | ((OPGRP_ALU_IMMED__F3_SLLI) << 12)
           | ((rs1_rd) << 7) | (OPGRP_ALU_IMMED);
}

static uint32_t expand_q2_lwsp(uint16_t instruction) {
    uint32_t rd = (instruction >> 7) & 0b11111;
    if (rd == 0) {
        return RV64_ILLEGAL_INSTRUCTION;
    }
    uint32_t imm = ((instruction >> 2) & 0x3) << 6 | ((instruction >> 12) & 0x1) << 5
                   | ((instruction >> 4) & 0x7) << 2;
    return (imm << 20) | (RV64_REG_SP << 15)  // hard coded to stack pointer
           | (OPGRP_LOAD__F3_LW << 12) | (rd << 7) | OPGRP_LOAD;
}

static uint32_t expand_q2_ldsp(uint16_t instruction) {
    uint32_t rd = (instruction >> 7) & 0b11111;
    if (rd == 0) {
        return RV64_ILLEGAL_INSTRUCTION;
    }
    uint32_t imm = ((instruction >> 2) & 0x7) << 6 | ((instruction >> 12) & 0x1) << 5
                   | ((instruction >> 5) & 0x3) << 3;
    return (imm << 20) | (RV64_REG_SP << 15) | (OPGRP_LOAD__F3_LD << 12) | (rd << 7) | OPGRP_LOAD;
}

static uint32_t expand_q2_jr_mv(uint16_t instruction) {
    uint8_t rs2 = (instruction >> 2) & 0b11111;
    uint8_t rs1 = (instruction >> 7) & 0b11111;
    if (rs1 == 0) {
        return RV64_ILLEGAL_INSTRUCTION;
    }
    if (rs2 == 0) {
        return (0 << 20) | (rs1 << 15) | (0 << 12) | (0 << 7) | OP_JALR;
    } else {
        return (0 << 25) | (rs2 << 20) | (0 << 15) | (0 << 12) | (rs1 << 7) | 0x33;
    }
}

static constexpr uint32_t RV64_EBREAK = (1 << 20) | (0 << 15) | (0 << 12) | (0 << 7) | 0x73;

static uint32_t expand_q2_ebreak_jalr_add(uint16_t instruction) {
    uint8_t rs2 = (instruction >> 2) & 0b11111;
    uint8_t rs1 = (instruction >> 7) & 0b11111;
    if (rs1 == 0 && rs2 == 0) {
        return RV64_EBREAK;
    }

    if (rs1 != 0 && rs2 == 0) {
        return (0 << 20) | (rs1 << 15) | (0 << 12) | (1 << 7) | OP_JALR;
    }

    if (rs1 != 0 && rs2 != 0) {
        return (0x00 << 25) | (rs2 << 20) | (rs1 << 15) | (0x0 << 12) | (rs1 << 7) | 0x33;
    }

    return RV64_ILLEGAL_INSTRUCTION;
}

static uint32_t expand_q2_swsp(uint16_t instruction) {
    uint32_t rs2 = (instruction >> 2) & 0b11111;
    uint32_t offset = ((instruction >> 9) & 0xF) << 2 | ((instruction >> 7) & 0x3) << 6;
    uint32_t imm_hi = (offset >> 5) & 0b1111111;
    uint32_t imm_lo = offset & 0b11111;
    return (imm_hi << 25) | (rs2 << 20) | (RV64_REG_SP << 15) | (0x2 << 12) | (imm_lo << 7)
           | OPGRP_STORE;
}

static uint32_t expand_q2_sdsp(uint16_t instruction) {
    uint32_t rs2 = (instruction >> 2) & 0b11111;
    uint32_t offset = ((instruction >> 10) & 0x7) << 3 | ((instruction >> 7) & 0x7) << 6;
    uint32_t imm_hi = (offset >> 5) & 0b1111111;
    uint32_t imm_lo = offset & 0b11111;
    return (imm_hi << 25) | (rs2 << 20) | (RV64_REG_SP << 15) | (0x3 << 12) | (imm_lo << 7)
           | OPGRP_STORE;
}

// MARK: Q3 - (not compressed, used to denote a full 32 bit instruction)
constexpr uint8_t OPGRP_CQ3 = 0b11;

// MARK: Expand

/**
 * @param instruction Instruction to expand
 */
uint32_t rv64c_expand(uint16_t instruction) {
    uint32_t op = instruction & 0b11;
    uint32_t funct3 = (instruction >> 13) & 0b111;
    uint32_t expanded = 0;

    switch (op) {
        case OPGRP_CQ0: {
            switch (funct3) {
                // 0b000 - illegal instruction
                // case 0b000: addi4spn, but it's the same as illegal op, how do we differentiate?
                // case OPGRP_CQ0__F3_FLD: return expand_load_store_fld_ld_fsd_sd(instruction,
                // RV64_OPCODE_GROUP_FLOAT_LOAD, RV64_OPCODE_GROUP_FLOAT_LOAD_FUNCT3_FLD);//return
                // expand_c_fld(instruction); // todo - depends on rv64d
                case OPGRP_CQ0__F3_LW:
                    return expand_q0_lw(instruction);
                case OPGRP_CQ0__F3_LD:
                    return expand_q0_ld(instruction, OPGRP_LOAD, OPGRP_LOAD__F3_LD);
                // 0b100 - reserved
                // case OPGRP_CQ0__F3_FSD: return expand_c_fsd(instruction); // todo - depends on
                // rv64d
                case OPGRP_CQ0__F3_SW:
                    return expand_q0_sw(instruction);
                case OPGRP_CQ0__F3_SD:
                    return expand_q0_sd(instruction);
                default:
                    return RV64_ILLEGAL_INSTRUCTION;
            }
        }
        case OPGRP_CQ1: {
            switch (funct3) {
                // case C_OP_ADDI: return expand_c_addi(instruction);
                case OPGRP_CQ1__F3_ADDI:
                    return expand_q1_addi_addiw_li(
                        instruction, OPGRP_ALU_IMMED, OPGRP_ALU_IMMED__F3_ADDI
                    );
                // case 0b001: return expand_jump_jal_j(instruction); // jal, rv32 only
                case OPGRP_CQ1__F3_ADDIW:
                    return expand_q1_addi_addiw_li(
                        instruction, OPGRP_IMMED_WIDE, OPGRP_IMMED_WIDE__F3_ADDIW
                    );
                case OPGRP_CQ1__F3_LI:
                    return expand_q1_li(instruction);
                case OPGRP_CQ1__F3GRP_ADDI16SP_LUI:
                    return expand_q1_addi16sp_lui(instruction);
                case OPGRP_CQ1__F3GRP_ALU: {
                    uint32_t funct2 = (instruction >> 10) & 0b11;
                    switch (funct2) {
                        case OPGRP_CQ1__F3GRP_ALU__F2_SRLI:
                            return expand_q1_srli(instruction);
                        case OPGRP_CQ1__F3GRP_ALU__F2_SRAI:
                            return expand_q1_srai(instruction);
                        case OPGRP_CQ1__F3GRP_ALU__F2_ANDI:
                            return expand_q1_andi(instruction);
                        case OPGRP_CQ1__F3GRP_ALU__F2GRP_LOGICAL: {
                            uint8_t funct1 = (instruction >> 12) & 0b1;
                            switch (funct1) {
                                case OPGRP_CQ1__F3GRP_ALU__F2GRP_LOGICAL__F1GRP_WORD: {
                                    uint8_t funct2_2 = (instruction >> 5) & 0b11;
                                    switch (funct2_2) {
                                        case OPGRP_CQ1__F3GRP_ALU__F2GRP_LOGICAL__F1GRP_WORD__F2_SUB:
                                            return expand_q1_sub(instruction);
                                        case OPGRP_CQ1__F3GRP_ALU__F2GRP_LOGICAL__F1GRP_WORD__F2_XOR:
                                            return expand_q1_xor(instruction);
                                        case OPGRP_CQ1__F3GRP_ALU__F2GRP_LOGICAL__F1GRP_WORD__F2_OR:
                                            return expand_q1_or(instruction);
                                        case OPGRP_CQ1__F3GRP_ALU__F2GRP_LOGICAL__F1GRP_WORD__F2_AND:
                                            return expand_q1_and(instruction);
                                        default:
                                            return RV64_ILLEGAL_INSTRUCTION;
                                    }
                                }
                                case OPGRP_CQ1__F3GRP_ALU__F2GRP_LOGICAL__F1GRP_WIDE: {
                                    uint8_t funct2_2 = (instruction >> 5) & 0b11;
                                    switch (funct2_2) {
                                        case OPGRP_CQ1__F3GRP_ALU__F2GRP_LOGICAL__F1GRP_WIDE__F2_SUBW:
                                            return expand_q1_subw(instruction);
                                        case OPGRP_CQ1__F3GRP_ALU__F2GRP_LOGICAL__F1GRP_WIDE__F2_ADDW:
                                            return expand_q1_addw(instruction);
                                        default:
                                            return RV64_ILLEGAL_INSTRUCTION;
                                    }
                                }
                                default:
                                    return RV64_ILLEGAL_INSTRUCTION;
                            }
                        }
                        default:
                            return RV64_ILLEGAL_INSTRUCTION;
                    }
                }
                case OPGRP_CQ1__F3_J:
                    return expand_q1_j(instruction);
                case OPGRP_CQ1__F3_BEQZ:
                    return expand_q1_beqz(instruction);
                case OPGRP_CQ1__F3_BNEZ:
                    return expand_q1_bnez(instruction);
                default:
                    return RV64_ILLEGAL_INSTRUCTION;
            }
        }
        case OPGRP_CQ2: {
            switch (funct3) {
                case OPGRP_CQ2__F3_SLLI:
                    return expand_q2_slli(instruction);
                case OPGRP_CQ2__F3_FLDSP:
                    return 0;  // todo - depends on rv64d
                case OPGRP_CQ2__F3_LWSP:
                    return expand_q2_lwsp(instruction);
                case OPGRP_CQ2__F3_LDSP:
                    return expand_q2_ldsp(instruction);
                case OPGRP_CQ2__F3GRP_JUMP: {
                    uint8_t funct1 = (instruction >> 12) & 0b1;
                    switch (funct1) {
                        case 0b0:
                            return expand_q2_jr_mv(instruction);
                        case 0b1:
                            return expand_q2_ebreak_jalr_add(instruction);
                        default:
                            return RV64_ILLEGAL_INSTRUCTION;
                    }
                    // TODO: JR, MV, EBREAK, JALR, ADD
                    return 0;
                }
                case OPGRP_CQ2__F3_FSDSP:
                    return 0;  // todo - depends on rv64d
                case OPGRP_CQ2__F3_SWSP:
                    return expand_q2_swsp(instruction);
                case OPGRP_CQ2__F3_SDSP:
                    return expand_q2_sdsp(instruction);
                default:
                    return RV64_ILLEGAL_INSTRUCTION;
            }
            break;
        }
        case OPGRP_CQ3: {
            // shouldn't be called, 0b11 is for non-compressed instructions.
            break;
        }
    }
    return (expanded);
}
