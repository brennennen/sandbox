
#include <stdint.h>

#include "logger.h"

#include "rv64/rv64_decode.h"
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"

#include "rv64/instructions/rv64_register_immediate.h"

emu_result_t rv64_disassemble_register_immediate(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    int16_t imm12 = 0;
    uint8_t rs1 = 0;
    uint8_t rd = 0;

    rv64_decode_register_immediate(raw_instruction, &imm12, &rs1, &rd);

    char* rs1_name = emu_rv64_map_register_name(rs1);
    char* rd_name = emu_rv64_map_register_name(rd);
    char* tag_name = rv64_instruction_tag_mnemonic[tag];

    int written = snprintf(buffer + *index, buffer_size - *index,
        "%s %s, %s, %d", tag_name, rd_name, rs1_name, imm12);
    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
    return(ER_SUCCESS);
}

static inline void rv64_jalr(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] + imm12;
}

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

static inline void rv64_slli(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] << imm12;
}

static inline void rv64_srli(emulator_rv64_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    // TODO: lookup arithmetic vs logical shift
    // emulator->registers.regs[rd] = emulator->registers.regs[rs1] >> imm12;
}

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
