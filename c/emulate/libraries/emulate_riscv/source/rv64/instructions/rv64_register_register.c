
#include <stdint.h>
#include <assert.h>

#include "logger.h"

#include "rv64/rv64_decode.h"
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"

#include "rv64/instructions/rv64_register_immediate.h"

emu_result_t rv64_disassemble_register_register(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    uint8_t rs2 = 0;
    uint8_t rs1 = 0;
    uint8_t rd = 0;

    rv64_decode_register_register(raw_instruction, &rs2, &rs1, &rd);

    char* rs1_name = emu_rv64_map_register_name(rs1);
    char* rs2_name = emu_rv64_map_register_name(rs2);
    char* rd_name = emu_rv64_map_register_name(rd);
    char* tag_name = rv64_instruction_tag_mnemonic[tag];

    int written = snprintf(buffer + *index, buffer_size - *index,
        "%s %s, %s, %s", tag_name, rd_name, rs1_name, rs2_name);

    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
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

/**
 * MUL performs a 64 bit by 64 bit multiplication of rs1 by rs2 and places the lower 64 bits
 * in the destination register.
 * 12.1 Multiplication Operations
 */
static inline void rv64m_mul(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] * emulator->registers.regs[rs2];
}
// commented out code = uncertain and untested

/**
 * MULH performs a 64 bit by 64 bit multiplication of rs1 by rs2 into a 128 bit result
 * and places the high 64 bits in the destination register.
 * 12.1 Multiplication Operations
 */
static inline void rv64m_mulh(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
#if defined(__GNUC__) || defined(__clang)
    __int128_t product = (__int128_t)emulator->registers.regs[rs1] * (__int128_t)emulator->registers.regs[rs2];
    emulator->registers.regs[rd] = (uint64_t)(product >> 64);
#else
    // TODO: fallback
    LOG(LOG_ERROR, "rv64m_mulh: fallback not implemeneted for non gcc/clang compilers.");
    assert(1 == 0);
#endif

    // emulator->registers.regs[rd] = emulator->registers.regs[rs1] * emulator->registers.regs[rs2];

    // something like: emulator->registers.regs[rd] = emulator->registers.regs[rs1] * emulator->registers.regs[rs2];
}

static inline void rv64m_mulhsu(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {

}

static inline void rv64m_mulhu(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
#if defined(__GNUC__) || defined(__clang)
    __int128_t product = (__int128_t)emulator->registers.regs[rs1] * (__int128_t)emulator->registers.regs[rs2];
    emulator->registers.regs[rd] = (uint64_t)(product >> 64);
#else
    // TODO: fallback
    LOG(LOG_ERROR, "rv64m_mulhu: fallback not implemeneted for non gcc/clang compilers.");
    assert(1 == 0);
#endif
}

static inline void rv64m_div(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    // emulator->registers.regs[rd] = emulator->registers.regs[rs1] / emulator->registers.regs[rs2];
}

static inline void rv64m_divu(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    // emulator->registers.regs[rd] = emulator->registers.regs[rs1] / emulator->registers.regs[rs2];
}

static inline void rv64m_rem(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    // emulator->registers.regs[rd] = emulator->registers.regs[rs1] % emulator->registers.regs[rs2];
}

static inline void rv64m_remu(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {

}

static inline void rv64m_mulw(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    // int32_t rs1_value = emulator->registers.regs[rs1];
    // int32_t rs2_value = emulator->registers.regs[rs2];
    // emulator->registers.regs[rd] = (int32_t) rs1_value * rs2_value;
}

static inline void rv64m_divw(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    // int32_t rs1_value = emulator->registers.regs[rs1];
    // int32_t rs2_value = emulator->registers.regs[rs2];
    // emulator->registers.regs[rd] = (int32_t) rs1_value / rs2_value;
}

static inline void rv64m_divuw(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    // uint32_t rs1_value = emulator->registers.regs[rs1];
    // uint32_t rs2_value = emulator->registers.regs[rs2];
    // emulator->registers.regs[rd] = (uint32_t) rs1_value / rs2_value;
}

// "REMW and REMUW are RV64 instructions that provide the corresponding signed and unsigned
// remainder operations. Both REMW and REMUW always sign-extend the 32-bit result to 64 bits,
// including on a divide by zero."
static inline void rv64m_remw(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    // int32_t rs1_value = emulator->registers.regs[rs1];
    // int32_t rs2_value = emulator->registers.regs[rs2];
    // emulator->registers.regs[rd] = (int32_t) rs1_value % rs2_value;
}

// "REMW and REMUW are RV64 instructions that provide the corresponding signed and unsigned
// remainder operations. Both REMW and REMUW always sign-extend the 32-bit result to 64 bits,
// including on a divide by zero."
static inline void rv64m_remuw(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    uint32_t rs1_value = emulator->registers.regs[rs1];
    uint32_t rs2_value = emulator->registers.regs[rs2];
    emulator->registers.regs[rd] = (uint32_t) rs1_value % rs2_value;
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
        case I_RV64M_MUL: {
            rv64m_mul(emulator, rs1, rs2, rd);
            break;
        }
        case I_RV64M_MULH: {
            rv64m_mulh(emulator, rs1, rs2, rd);
            break;
        }
        case I_RV64M_MULHSU: {
            rv64m_mulhsu(emulator, rs1, rs2, rd);
            break;
        }
        case I_RV64M_MULHU: {
            rv64m_mulhu(emulator, rs1, rs2, rd);
            break;
        }
        case I_RV64M_DIV: {
            rv64m_div(emulator, rs1, rs2, rd);
            break;
        }
        case I_RV64M_DIVU: {
            rv64m_divu(emulator, rs1, rs2, rd);
            break;
        }
        case I_RV64M_REM: {
            rv64m_rem(emulator, rs1, rs2, rd);
            break;
        }
        case I_RV64M_REMU: {
            rv64m_remu(emulator, rs1, rs2, rd);
            break;
        }
        case I_RV64M_MULW: {
            rv64m_mulw(emulator, rs1, rs2, rd);
            break;
        }
        case I_RV64M_DIVW: {
            rv64m_divw(emulator, rs1, rs2, rd);
            break;
        }
        case I_RV64M_DIVUW: {
            rv64m_divuw(emulator, rs1, rs2, rd);
            break;
        }
        case I_RV64M_REMW: {
            rv64m_remw(emulator, rs1, rs2, rd);
            break;
        }
        case I_RV64M_REMUW: {
            rv64m_remuw(emulator, rs1, rs2, rd);
            break;
        }
        default: {
            LOG(LOG_ERROR, "rv64i_emulate_register_register: instruction not implemented");
            return(ER_FAILURE);
        }
    }
    return(ER_SUCCESS);
}
