
#include <stdint.h>

#include "rv64i/rv64i_decode.h"
#include "rv64i/rv64i_emulate.h"
#include "rv64i/rv64i_instructions.h"

#include "rv64i/instructions/rv64i_register_immediate.h"

emu_result_t rv64i_disassemble_register_immediate(
    emulator_rv64i_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64i_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    int16_t imm12 = 0;
    uint8_t rs1 = 0;
    uint8_t rd = 0;

    rv64i_decode_register_immediate(raw_instruction, &imm12, &rs1, &rd);

    char* rs1_name = emu_rv64i_map_register_name(rs1);
    char* rd_name = emu_rv64i_map_register_name(rd);
    char* tag_name = rv64i_instruction_tag_mnemonic[tag];

    int written = snprintf(buffer + *index, buffer_size - *index,
        "%s %s, %s, %d", tag_name, rd_name, rs1_name, imm12);
    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
    return(ER_SUCCESS);
}

static inline void rv64i_jalr(emulator_rv64i_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] + imm12;
}

static inline void rv64i_addi(emulator_rv64i_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] + imm12;
}

static inline void rv64i_slti(emulator_rv64i_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    if (emulator->registers.regs[rs1] < imm12) {
        emulator->registers.regs[rd] = 1;
    } else {
        emulator->registers.regs[rd] = 0;
    }
}

static inline void rv64i_sltiu(emulator_rv64i_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    if ((uint64_t)emulator->registers.regs[rs1] < (uint16_t) imm12) {
        emulator->registers.regs[rd] = 1;
    } else {
        emulator->registers.regs[rd] = 0;
    }
}

static inline void rv64i_xori(emulator_rv64i_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] ^ imm12;
}

static inline void rv64i_ori(emulator_rv64i_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] | imm12;
}

static inline void rv64i_andi(emulator_rv64i_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] & imm12;
}

static inline void rv64i_slli(emulator_rv64i_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] << imm12;
}

static inline void rv64i_srli(emulator_rv64i_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    // TODO: lookup arithmetic vs logical shift
    // emulator->registers.regs[rd] = emulator->registers.regs[rs1] >> imm12;
}

static inline void rv64i_srai(emulator_rv64i_t* emulator, int16_t imm12, uint8_t rs1, uint8_t rd) {
    // TODO: lookup arithmetic vs logical shift
    // emulator->registers.regs[rd] = emulator->registers.regs[rs1] >> imm12;
}


emu_result_t rv64i_emulate_register_immediate(
    emulator_rv64i_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64i_t tag
) {
    int16_t imm12 = 0;
    uint8_t rs1 = 0;
    uint8_t rd = 0;

    rv64i_decode_register_immediate(raw_instruction, &imm12, &rs1, &rd);

    switch(tag) {
        case I_RV64I_JALR: {
            rv64i_jalr(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_ADDI: {
            rv64i_addi(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_SLTI: {
            rv64i_slti(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_SLTIU: {
            rv64i_sltiu(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_XORI: {
            rv64i_xori(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_ORI: {
            rv64i_ori(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_ANDI: {
            rv64i_andi(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_SLLI: {
            rv64i_slli(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_SRLI: {
            rv64i_srli(emulator, imm12, rs1, rd);
            break;
        }
        case I_RV64I_SRAI:
        {
            rv64i_srai(emulator, imm12, rs1, rd);
            break;
        }
    }
    return(ER_SUCCESS);
}
