
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
    // pc is incremented when the instruction is originally read, so need to decrement here.
    emulator->registers.regs[rd] = (imm20 << 12) + emulator->registers.pc - 4;
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

static inline void rv64i_jal(emulator_rv64_t* emulator, int32_t offset, uint8_t rd) {
    if (rd != 0) {
        emulator->registers.regs[rd] = emulator->registers.pc; // point rd to the next instruction
    }
    emulator->registers.pc = emulator->registers.pc + offset - 4;
    LOGD("%s: offset: %d (pc: %ld), rd: %d", __func__, offset, emulator->registers.pc, rd);

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

static inline void rv64i_jalr(emulator_rv64_t* emulator, int32_t imm12, uint8_t rs1, uint8_t rd) {
    if (rd != 0) {
        emulator->registers.regs[rd] = emulator->registers.pc; // point rd to the next instruction
    }
    LOGD("%s: imm12: %d, rs1: %ld (%d), rd: %d", __func__,
        imm12, emulator->registers.regs[rs1], rs1, rd);
    // TODO: does imm12 need to be left shifted 1 bit?
    emulator->registers.pc = emulator->registers.regs[rs1] + imm12;
}

/**
 * BEQ - Branch if EQual. If rs1 and rs2 are equal, add the offset to pc (branch out of
 * the mainline execution flow).
 * `beq rs1, rs2, <label>`
 * `beq rs1, rs2, . + <jump offset>`
 * pseudo-instruction beqz: rs2 is zero and ommitted `beqz rs1, <label>`.
 */
static inline void rv64i_beq(emulator_rv64_t* emulator, int32_t offset, uint8_t rs1, uint8_t rs2) {
    LOGD("%s: offset: %d, rs1: %ld, rs2: %ld", __func__, offset,
        emulator->registers.regs[rs1], emulator->registers.regs[rs2]);
    if (emulator->registers.regs[rs1] == emulator->registers.regs[rs2]) {
        emulator->registers.pc = emulator->registers.pc + offset - 4;
    }
}

static inline void rv64i_bne(emulator_rv64_t* emulator, int32_t offset, uint8_t rs1, uint8_t rs2) {
    LOGD("%s: offset: %d, rs1: %ld, rs2: %ld", __func__, offset,
        emulator->registers.regs[rs1], emulator->registers.regs[rs2]);
    if (emulator->registers.regs[rs1] != emulator->registers.regs[rs2]) {
        emulator->registers.pc = emulator->registers.pc + offset - 4;
    }
}

static inline void rv64i_blt(emulator_rv64_t* emulator, int32_t offset, uint8_t rs1, uint8_t rs2) {
    LOGD("%s: offset: %d, rs1: %ld, rs2: %ld", __func__, offset,
        emulator->registers.regs[rs1], emulator->registers.regs[rs2]);
    if ((int64_t)emulator->registers.regs[rs1] < (int64_t)emulator->registers.regs[rs2]) {
        emulator->registers.pc = emulator->registers.pc + offset - 4;
    }
}

static inline void rv64i_bge(emulator_rv64_t* emulator, int32_t offset, uint8_t rs1, uint8_t rs2) {
    LOGD("%s: offset: %d, rs1: %ld, rs2: %ld", __func__, offset,
        emulator->registers.regs[rs1], emulator->registers.regs[rs2]);
    if ((int64_t)emulator->registers.regs[rs1] >= (int64_t)emulator->registers.regs[rs2]) {
        emulator->registers.pc = emulator->registers.pc + offset - 4;
    }
}

static inline void rv64i_bltu(emulator_rv64_t* emulator, int32_t offset, uint8_t rs1, uint8_t rs2) {
    LOGD("%s: offset: %d, rs1: %ld, rs2: %ld", __func__, offset,
        emulator->registers.regs[rs1], emulator->registers.regs[rs2]);
    if (emulator->registers.regs[rs1] < emulator->registers.regs[rs2]) {
        emulator->registers.pc = emulator->registers.pc + offset - 4;
    }
}

static inline void rv64i_bgeu(emulator_rv64_t* emulator, int32_t offset, uint8_t rs1, uint8_t rs2) {
    LOGD("%s: offset: %d, rs1: %ld, rs2: %ld", __func__, offset,
        emulator->registers.regs[rs1], emulator->registers.regs[rs2]);
    if (emulator->registers.regs[rs1] >= emulator->registers.regs[rs2]) {
        emulator->registers.pc = emulator->registers.pc + offset - 4;
    }
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

static inline void rv64i_sb(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rs2) {
    uint64_t address = emulator->registers.regs[rs1] + imm12;
    emulator->memory[address] = emulator->registers.regs[rs2] & 0xFF;
}

static inline void rv64i_sh(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rs2) {
    uint64_t address = emulator->registers.regs[rs1] + imm12;
    // todo: just use memcpy? uses host machine endieness?
    emulator->memory[address] = (uint8_t) (emulator->registers.regs[rs2] & 0xFF);
    emulator->memory[address + 1] = (uint8_t) ((emulator->registers.regs[rs2] >> 8) & 0xFF);
}

static inline void rv64i_sw(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rs2) {
    uint64_t address = emulator->registers.regs[rs1] + imm12;
    emulator->memory[address] = (uint8_t) (emulator->registers.regs[rs2] & 0xFF);
    emulator->memory[address + 1] = (uint8_t) ((emulator->registers.regs[rs2] >> 8) & 0xFF);
    emulator->memory[address + 2] = (uint8_t) ((emulator->registers.regs[rs2] >> 16) & 0xFF);
    emulator->memory[address + 3] = (uint8_t) ((emulator->registers.regs[rs2] >> 24) & 0xFF);
}

static emu_result_t rv64i_emulate_s_type(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    uint16_t offset = 0;
    uint8_t rs1 = 0;
    uint8_t rs2 = 0;

    rv64_decode_store(raw_instruction, &offset, &rs1, &rs2);

    switch(tag) {
        case I_RV64I_SB: {
            rv64i_sb(emulator, offset, rs1, rs2);
            break;
        }
        case I_RV64I_SH: {
            rv64i_sh(emulator, offset, rs1, rs2);
            break;
        }
        case I_RV64I_SW: {
            rv64i_sw(emulator, offset, rs1, rs2);
            break;
        }
        default: {
            LOG(LOG_ERROR, "%s: instruction not implemented", __func__);
            return(ER_FAILURE);
        }
    }
    return(ER_SUCCESS);
}


static inline void rv64i_lb(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    uint64_t address = emulator->registers.regs[rs1] + imm12;
    emulator->registers.regs[rd] = (int8_t) emulator->memory[address];
}

static inline void rv64i_lh(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    uint64_t address = emulator->registers.regs[rs1] + imm12;
    uint16_t halfword = (uint16_t)emulator->memory[address] |
        ((uint16_t) emulator->memory[address + 1] << 8);
    emulator->registers.regs[rd] = (int16_t) halfword;
}

static inline void rv64i_lw(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    uint64_t address = emulator->registers.regs[rs1] + imm12;
    uint32_t word = (uint32_t)emulator->memory[address] |
        ((uint32_t) emulator->memory[address + 1] << 24) |
        ((uint32_t) emulator->memory[address + 2] << 16) |
        ((uint32_t) emulator->memory[address + 3] << 8);
    emulator->registers.regs[rd] = (int32_t) word;
}

static inline void rv64i_lbu(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    uint64_t address = emulator->registers.regs[rs1] + imm12;
    emulator->registers.regs[rd] = emulator->memory[address];
}

static inline void rv64i_lhu(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    uint64_t address = emulator->registers.regs[rs1] + imm12;
    emulator->registers.regs[rd] = (uint16_t)emulator->memory[address] |
        ((uint16_t) emulator->memory[address + 1] << 8);
}

/**
 * NOTE: NOP is encoded as `ADDI x0, x0, 0`.
 */
static inline void rv64_addi(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] + imm12;
}

/**
 * "Set Less Than Immediate" - Set rd to 1 if rs1 is less than the provided immediate when
 * both numbers are treated as signed.
 */
static inline void rv64_slti(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    if ((int64_t)emulator->registers.regs[rs1] < (int64_t)imm12) {
        emulator->registers.regs[rd] = 1;
    } else {
        emulator->registers.regs[rd] = 0;
    }
    LOGD("%s: rs1: %d, imm: %d, rd: %d", __func__, emulator->registers.regs[rs1],
         imm12, emulator->registers.regs[rd]);
}

/**
 * "Set Less Than Immediate Unsigned" - Set rd to 1 if rs1 is less than the immedate when
 * both numbers are treated as unsigned.
 */
static inline void rv64_sltiu(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    if ((uint64_t)emulator->registers.regs[rs1] < (uint16_t) imm12) {
        emulator->registers.regs[rd] = 1;
    } else {
        emulator->registers.regs[rd] = 0;
    }
    LOGD("%s: rs1: %d, imm: %d, rd: %d", __func__, emulator->registers.regs[rs1],
         imm12, emulator->registers.regs[rd]);
}

static inline void rv64_xori(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] ^ imm12;
}

/**
 * "logical/bitwise OR" - Both numbers sign extended.
 */
static inline void rv64_ori(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] | imm12;
}

/**
 * "logical/bitwise AND" - Both numbers sign extended.
 */
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
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] >> ((uint16_t)imm12);
}

/**
 * SRAI - Shift Right Arithmetic with Immediate.
 * The original sign bit is copied into the vacated upper bits.
 * (Section 2.4.1. Integer Register-Immediate Instructions)
 */
static inline void rv64_srai(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    emulator->registers.regs[rd] = ((int64_t)emulator->registers.regs[rs1]) >> imm12;
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
        case I_RV64I_LB: {
            rv64i_lb(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_LH: {
            rv64i_lh(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_LW: {
            rv64i_lw(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_LBU: {
            rv64i_lbu(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_LHU: {
            rv64i_lhu(emulator, imm12, rs1, rd);
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
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] << emulator->registers.regs[rs2];
}

/**
 * "Set Less Than" - Set rd to 1 if rs1 is less than rs2 when both registers are treated
 * as signed numbers.
 */
static inline void rv64i_slt(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    if (emulator->registers.regs[rs1] < emulator->registers.regs[rs2]) {
        emulator->registers.regs[rd] = 1;
    } else {
        emulator->registers.regs[rd] = 0;
    }
}

/**
 * "Set Less Than Unsigned" - Set rd to 1 if rs1 is less than rs2 when both registers
 * are treated as unsigned numbers.
 */
static inline void rv64i_sltu(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    if ((uint64_t)emulator->registers.regs[rs1] < (uint64_t)emulator->registers.regs[rs2]) {
        emulator->registers.regs[rd] = 1;
    } else {
        emulator->registers.regs[rd] = 0;
    }
}

static inline void rv64i_xor(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] ^ emulator->registers.regs[rs2];
}

// c shift right is logical when unsigned, arithmetic when signed (sign extension)
static inline void rv64i_srl(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] >> emulator->registers.regs[rs2];
}

static inline void rv64i_sra(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    emulator->registers.regs[rd] = ((int64_t)emulator->registers.regs[rs1]) >>
        ((int64_t)emulator->registers.regs[rs2]);
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
        case I_RV64I_JALR: {
            result = rv64_emulate_register_immediate(emulator, raw_instruction, tag);
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

        // Core Format "S" - "store"
        case I_RV64I_SB:
        case I_RV64I_SH:
        case I_RV64I_SW: {
            result = rv64i_emulate_s_type(emulator, raw_instruction, tag);
            break;
        }

        // Core Format "I" - "register-immediate"
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
        case I_RV64I_ANDI:
        case I_RV64I_SLLI:
        case I_RV64I_SRLI:
        case I_RV64I_SRAI: {
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
        case I_RV64I_SRA:
        case I_RV64I_OR:
        case I_RV64I_AND: {
            result = rv64_emulate_register_register(emulator, raw_instruction, tag);
            break;
        }
    }
    return result;
}
