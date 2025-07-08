
#include <stdint.h>

#include "logger.h"

#include "rv64/rv64_decode.h"
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"
#include "rv64/rv64_decode.h"

#include "rv64/instructions/rv64i_base_integer.h"



static inline void rv64_jalr(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    // TODO
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

emu_result_t rv64_emulate_register_immediate(
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
            rv64_jalr(emulator, imm12, rs1, rd);
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

emu_result_t rv64_emulate_register_register(
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
        case I_RV64I_LUI: {
            // TODO
            result = ER_FAILURE;
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
