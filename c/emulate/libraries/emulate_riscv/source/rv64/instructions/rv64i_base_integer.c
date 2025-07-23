
#include <stdint.h>

#include "logger.h"

#include "rv64/rv64_decode.h"
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"
#include "rv64/rv64_decode.h"

#include "rv64/instructions/rv64i_base_integer.h"

/**
 * LUI - Load Upper Immediate. Places the 32-bi9t U-immediate into register rd,
 * filling the lowest 12 bits with zeros. The 32-bit result is sign-extended to
 * 64 bits.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_integer_register_immediate_instructions_2
 */
static inline void rv64i_lui(emulator_rv64_t* emulator, int32_t imm20, uint8_t rd) {
    emulator->registers.regs[rd] = imm20 << 12;
}

/**
 * AUIPC - Add Upper Immediate to `pc`. Used to build pc-relative addresses and uses
 * the "U-Type" format. AUIPC forms a 32 bit offset from the U-immediate, filling in
 * the lowest 12 bits with zeros, adds this offset to the address of the AUIPC
 * instruction, then places the result in register rd.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_integer_register_immediate_instructions_2
 */
static inline void rv64i_auipc(emulator_rv64_t* emulator, int32_t imm20, uint8_t rd) {
    emulator->registers.regs[rd] = (imm20 << 12) + emulator->registers.pc;
}

static emu_result_t rv64i_emulate_upper_immediate(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    int32_t imm20 = 0;
    uint8_t rd = 0;

    rv64_decode_upper_immediate(raw_instruction, &imm20, &rd);

    switch(tag) {
        case I_RV64I_LUI: {
            rv64i_lui(emulator, imm20, rd);
            break;
        }
        case I_RV64I_AUIPC: {
            rv64i_auipc(emulator, imm20, rd);
            break;
        }
        default: {
            LOG(LOG_ERROR, "rv64i_emulate_upper_immediate: instruction not implemented");
            return(ER_FAILURE);
        }
    }
    return(ER_SUCCESS);
}

static inline void rv64i_jal(emulator_rv64_t* emulator, int32_t imm20, uint8_t rd) {
    printf("todo: rv64i_jal\n");
}

static inline void rv64i_jalr(emulator_rv64_t* emulator, int32_t imm12, uint8_t rs1, uint8_t rd) {
    printf("todo: rv64i_jalr\n");
}

static emu_result_t rv64i_emulate_j_type(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    int32_t offset = 0;
    uint8_t rd = 0;

    rv64_decode_j_type(raw_instruction, &offset, &rd);

    switch(tag) {
        case I_RV64I_JAL: {
            rv64i_jal(emulator, offset, rd);
            break;
        }
        default: {
            LOG(LOG_ERROR, "rv64i_emulate_j_type: instruction not implemented");
            return(ER_FAILURE);
        }
    }
    return(ER_SUCCESS);
}

static inline void rv64i_beq(emulator_rv64_t* emulator, int32_t offset, uint8_t rs1, uint8_t rs2) {
    printf("todo: rv64i_beq\n");
}

static inline void rv64i_bne(emulator_rv64_t* emulator, int32_t offset, uint8_t rs1, uint8_t rs2) {
    printf("todo: rv64i_bne\n");
}

static inline void rv64i_blt(emulator_rv64_t* emulator, int32_t offset, uint8_t rs1, uint8_t rs2) {
    printf("todo: rv64i_blt\n");
}

static inline void rv64i_bge(emulator_rv64_t* emulator, int32_t offset, uint8_t rs1, uint8_t rs2) {
    printf("todo: rv64i_bge\n");
}

static inline void rv64i_bltu(emulator_rv64_t* emulator, int32_t offset, uint8_t rs1, uint8_t rs2) {
    printf("todo: rv64i_bltu\n");
}

static inline void rv64i_bgeu(emulator_rv64_t* emulator, int32_t offset, uint8_t rs1, uint8_t rs2) {
    printf("todo: rv64i_bgeu\n");
}

static emu_result_t rv64i_emulate_b_type(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    int32_t offset = 0;
    uint8_t rs1 = 0;
    uint8_t rs2 = 0;

    rv64_decode_branch(raw_instruction, &offset, &rs1, &rs2);

    switch(tag) {
        case I_RV64I_BEQ: {
            rv64i_beq(emulator, offset, rs1, rs2);
            break;
        }
        case I_RV64I_BNE: {
            rv64i_bne(emulator, offset, rs1, rs2);
            break;
        }
        case I_RV64I_BLT: {
            rv64i_blt(emulator, offset, rs1, rs2);
            break;
        }
        case I_RV64I_BGE: {
            rv64i_bge(emulator, offset, rs1, rs2);
            break;
        }
        case I_RV64I_BLTU: {
            rv64i_bltu(emulator, offset, rs1, rs2);
            break;
        }
        case I_RV64I_BGEU: {
            rv64i_bgeu(emulator, offset, rs1, rs2);
            break;
        }
        default: {
            LOG(LOG_ERROR, "rv64i_emulate_b_type: instruction not implemented");
            return(ER_FAILURE);
        }
    }
    return(ER_SUCCESS);
}


/**
 * NOTE: NOP is encoded as `ADDI x0, x0, 0`.
 */
static inline void rv64_addi(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] + imm12;
}

static inline void rv64_slti(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    if (emulator->registers.regs[rs1] < imm12) {
        emulator->registers.regs[rd] = 1;
    } else {
        emulator->registers.regs[rd] = 0;
    }
}

static inline void rv64_sltiu(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    if ((uint64_t)emulator->registers.regs[rs1] < (uint16_t) imm12) {
        emulator->registers.regs[rd] = 1;
    } else {
        emulator->registers.regs[rd] = 0;
    }
}

static inline void rv64_xori(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] ^ imm12;
}

static inline void rv64_ori(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] | imm12;
}

static inline void rv64_andi(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] & imm12;
}

/**
 * SLLI - Shift Logical Left with Immediate.
 * Zeros are shifted into the lower bits.
 * (Section 2.4.1. Integer Register-Immediate Instructions)
 */
static inline void rv64_slli(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] << imm12;
}

/**
 * SRLI - Shift Right Logical with Immediate.
 * Zeros are shifted into the upper bits.
 * (Section 2.4.1. Integer Register-Immediate Instructions)
 */
static inline void rv64_srli(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    // TODO: lookup arithmetic vs logical shift
    // emulator->registers.regs[rd] = emulator->registers.regs[rs1] >> imm12;
}

/**
 * SRAI - Shift Right Arithmetic with Immediate.
 * The original sign bit is copied into the vacated upper bits.
 * (Section 2.4.1. Integer Register-Immediate Instructions)
 */
static inline void rv64_srai(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    // TODO: lookup arithmetic vs logical shift
    // emulator->registers.regs[rd] = emulator->registers.regs[rs1] >> imm12;
}

static emu_result_t rv64_emulate_register_immediate(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    int16_t imm12 = 0;
    uint8_t rs1 = 0;
    uint8_t rd = 0;

    rv64_decode_register_immediate(raw_instruction, &imm12, &rs1, &rd);

    switch(tag) {
        case I_RV64I_JALR: {
            rv64i_jalr(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_ADDI: {
            rv64_addi(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_SLTI: {
            rv64_slti(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_SLTIU: {
            rv64_sltiu(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_XORI: {
            rv64_xori(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_ORI: {
            rv64_ori(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_ANDI: {
            rv64_andi(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_SLLI: {
            rv64_slli(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_SRLI: {
            rv64_srli(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_SRAI: {
            rv64_srai(emulator, imm12, rs1, rd);
            break;
        }
        default: {
            LOG(LOG_ERROR, "rv64i_emulate_register_immediate: instruction not implemented");
            return(ER_FAILURE);
        }
    }
    return(ER_SUCCESS);
}


static inline void rv64i_add(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] + emulator->registers.regs[rs2];
}

static inline void rv64i_sub(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] - emulator->registers.regs[rs2];
}

static inline void rv64i_sll(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {

}

static inline void rv64i_slt(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {

}

static inline void rv64i_sltu(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {

}

static inline void rv64i_xor(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] ^ emulator->registers.regs[rs2];
}

static inline void rv64i_srl(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {

}

static inline void rv64i_sra(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {

}

static inline void rv64i_or(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] | emulator->registers.regs[rs2];
}

static inline void rv64i_and(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] & emulator->registers.regs[rs2];
}

static emu_result_t rv64_emulate_register_register(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    uint8_t rs2 = 0;
    uint8_t rs1 = 0;
    uint8_t rd = 0;

    rv64_decode_register_register(raw_instruction, &rs2, &rs1, &rd);

    switch(tag) {
        case I_RV64I_ADD: {
            rv64i_add(emulator, rs1, rs2, rd);
            break;
        }
        case I_RV64I_SUB: {
            rv64i_sub(emulator, rs1, rs2, rd);
            break;
        }
        case I_RV64I_SLL: {
            rv64i_sll(emulator, rs1, rs2, rd);
            break;
        }
        case I_RV64I_SLT: {
            rv64i_slt(emulator, rs1, rs2, rd);
            break;
        }
        case I_RV64I_SLTU: {
            rv64i_sltu(emulator, rs1, rs2, rd);
            break;
        }
        case I_RV64I_XOR: {
            rv64i_xor(emulator, rs1, rs2, rd);
            break;
        }
        case I_RV64I_SRL: {
            rv64i_srl(emulator, rs1, rs2, rd);
            break;
        }
        case I_RV64I_SRA: {
            rv64i_sra(emulator, rs1, rs2, rd);
            break;
        }
        case I_RV64I_OR: {
            rv64i_or(emulator, rs1, rs2, rd);
            break;
        }
        case I_RV64I_AND: {
            rv64i_and(emulator, rs1, rs2, rd);
            break;
        }
        default: {
            LOG(LOG_ERROR, "rv64i_emulate_register_register: instruction not implemented");
            return(ER_FAILURE);
        }
    }
    return(ER_SUCCESS);
}

emu_result_t rv64i_base_integer_emulate(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    emu_result_t result = ER_FAILURE;
    switch(tag) {
        // Core Format "U" - "upper-immediate"
        case I_RV64I_LUI:
        case I_RV64I_AUIPC: {
            result = rv64i_emulate_upper_immediate(emulator, raw_instruction, tag);
            break;
        }
        // Core Format "J" - "jump"
        case I_RV64I_JAL: {
            result = rv64i_emulate_j_type(emulator, raw_instruction, tag);
            break;
        }
        // Core Format "B" - "branch"
        case I_RV64I_BEQ:
        case I_RV64I_BNE:
        case I_RV64I_BLT:
        case I_RV64I_BGE:
        case I_RV64I_BLTU:
        case I_RV64I_BGEU: {
            result = rv64i_emulate_b_type(emulator, raw_instruction, tag);
            break;
        }
        // Core Format "I" - "register-immediate"
        case I_RV64I_JALR:
        case I_RV64I_LB:
        case I_RV64I_LH:
        case I_RV64I_LW:
        case I_RV64I_LBU:
        case I_RV64I_LHU:
        case I_RV64I_ADDI:
        case I_RV64I_SLTI:
        case I_RV64I_SLTIU:
        case I_RV64I_XORI:
        case I_RV64I_ORI:
        case I_RV64I_ANDI: {
            result = rv64_emulate_register_immediate(emulator, raw_instruction, tag);
            break;
        }
        // Core Format "R" - "register-register"
        case I_RV64I_ADD:
        case I_RV64I_SUB:
        case I_RV64I_SLL:
        case I_RV64I_SLT:
        case I_RV64I_SLTU:
        case I_RV64I_XOR:
        case I_RV64I_SRL:
        case I_RV64I_OR:
        case I_RV64I_AND: {
            result = rv64_emulate_register_register(emulator, raw_instruction, tag);
            break;
        }
    }
    return result;
}
