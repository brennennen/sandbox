
#include <stdint.h>
#include <assert.h>

#include "logger.h"

#include "rv64/rv64_decode.h"
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"

#include "rv64/instructions/rv64m_multiplication.h"

/**
 * MUL performs a 64 bit by 64 bit multiplication of rs1 by rs2 and places the lower 64 bits
 * in the destination register.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_multiplication_operations
 */
static inline void rv64m_mul(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] * emulator->registers.regs[rs2];
}

/**
 * mulh - MULtiply High (Signed)
 * Multiplies a signed 64-bit value with a signed 64-bit value into a 128 bit result,
 * returns the upper 64 bits.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_multiplication_operations
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
}

/**
 * mulhsu - MULtiply High Signed by Unsigned
 * Multiplies a signed 64-bit value with an unsigned 64-bit value into a 128 bit result,
 * returns the upper 64 bits.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_multiplication_operations
 */
static inline void rv64m_mulhsu(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
#if defined(__GNUC__) || defined(__clang)
    // need casting for sign extension
    __int128_t product = (__int128_t)((int64_t)emulator->registers.regs[rs1] * (__uint128_t)emulator->registers.regs[rs2]);
    emulator->registers.regs[rd] = (int64_t)(product >> 64);
#else
    // TODO: fallback
    LOG(LOG_ERROR, "rv64m_mulhsu: fallback not implemeneted for non gcc/clang compilers.");
    assert(1 == 0);
#endif
}

/**
 * mulhu - MULtiply High Unsigned
 * Multiplies an unsigned 64-bit value with an unsigned 64-bit value into a 128 bit result,
 * returns the upper 64 bits.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_multiplication_operations
 */
static inline void rv64m_mulhu(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
#if defined(__GNUC__) || defined(__clang)
    __uint128_t product = (__uint128_t)emulator->registers.regs[rs1] * (__uint128_t)emulator->registers.regs[rs2];
    emulator->registers.regs[rd] = (uint64_t)(product >> 64);
#else
    // TODO: fallback
    LOG(LOG_ERROR, "rv64m_mulhu: fallback not implemeneted for non gcc/clang compilers.");
    assert(1 == 0);
#endif
}

/**
 * div - DIVide
 * Divides a signed 64-bit value with a signed 64-bit value into a 64 bit whole integer
 * result, rounding towards zero (use rem to get remainder).
 * dividend = divisor * quotient + remainder
 * Edge cases (table 11):
 * * Divide by 0: res = -1
 * * Signed division overflow: res = -2^(63)
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_division_operations
 */
static inline void rv64m_div(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    // divide by 0
    if (emulator->registers.regs[rs2] == 0) {
        emulator->registers.regs[rd] = -1;
    }
    // signed integer overflow
    else if ( emulator->registers.regs[rs1] == 0x8000000000000000 && \
                (int64_t)(emulator->registers.regs[rs2]) == -1)
    {
        emulator->registers.regs[rd] = 0x8000000000000000;
    }
    // common case
    else {
        emulator->registers.regs[rd] = ((int64_t)emulator->registers.regs[rs1]) / ((int64_t)emulator->registers.regs[rs2]);
    }
}

/**
 * divu - DIVide Unsigned
 * Divides an unsigned 64-bit value with an unsigned 64-bit value into a 64 bit whole integer
 * result, rounding towards zero (use rem to get remainder).
 * dividend = divisor * quotient + remainder
 * Edge cases (table 11):
 * * Divide by 0: res = max uint
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_division_operations
 */
static inline void rv64m_divu(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    // divide by 0
    if (emulator->registers.regs[rs2] == 0) {
        emulator->registers.regs[rd] = 0xFFFFFFFFFFFFFFFF; // 2^64 - 1
    }
    // common case
    else {
        emulator->registers.regs[rd] = emulator->registers.regs[rs1] / emulator->registers.regs[rs2];
    }
}

/**
 * rem - REMainder
 * For REM, the sign of a nonzero result equals the sign of the dividend.
 * dividend = divisor * quotient + remainder
 * Edge cases (table 11):
 * * Divide by 0: res = x
 * * Signed division overflow: res = 0
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_division_operations
 */
static inline void rv64m_rem(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    // divide by 0
    if (emulator->registers.regs[rs2] == 0) {
        emulator->registers.regs[rd] = emulator->registers.regs[rs1];
    }
    // signed division overflow
    else if ( emulator->registers.regs[rs1] == 0x8000000000000000 && \
                (int64_t)(emulator->registers.regs[rs2]) == -1)
    {
        emulator->registers.regs[rd] = 0;
    }
    // common case
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] % emulator->registers.regs[rs2];
}

/**
 * remu - REMainder Unsigned
 * For REM, the sign of a nonzero result equals the sign of the dividend.
 * dividend = divisor * quotient + remainder
 * Edge cases (table 11):
 * * Divide by 0: res = x
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_division_operations
 */
static inline void rv64m_remu(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    // divide by 0
    if (emulator->registers.regs[rs2] == 0) {
        emulator->registers.regs[rd] = emulator->registers.regs[rs1];
    }
    // common case
    emulator->registers.regs[rd] = emulator->registers.regs[rs1] % emulator->registers.regs[rs2];
}

/**
 * mulw - MULtiply Word
 * 32 bit mul
 */
static inline void rv64m_mulw(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    int32_t rs1_value = emulator->registers.regs[rs1];
    int32_t rs2_value = emulator->registers.regs[rs2];
    emulator->registers.regs[rd] = (int32_t) rs1_value * rs2_value;
}

static inline void rv64m_divw(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    int32_t rs1_value = emulator->registers.regs[rs1];
    int32_t rs2_value = emulator->registers.regs[rs2];
    emulator->registers.regs[rd] = (int32_t) rs1_value / rs2_value;
}

static inline void rv64m_divuw(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    uint32_t rs1_value = emulator->registers.regs[rs1];
    uint32_t rs2_value = emulator->registers.regs[rs2];
    emulator->registers.regs[rd] = (uint32_t) rs1_value / rs2_value;
}

// "REMW and REMUW are RV64 instructions that provide the corresponding signed and unsigned
// remainder operations. Both REMW and REMUW always sign-extend the 32-bit result to 64 bits,
// including on a divide by zero."
static inline void rv64m_remw(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    int32_t rs1_value = emulator->registers.regs[rs1];
    int32_t rs2_value = emulator->registers.regs[rs2];
    emulator->registers.regs[rd] = (int32_t) rs1_value % rs2_value;
}

// "REMW and REMUW are RV64 instructions that provide the corresponding signed and unsigned
// remainder operations. Both REMW and REMUW always sign-extend the 32-bit result to 64 bits,
// including on a divide by zero."
static inline void rv64m_remuw(emulator_rv64_t* emulator, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    uint32_t rs1_value = emulator->registers.regs[rs1];
    uint32_t rs2_value = emulator->registers.regs[rs2];
    emulator->registers.regs[rd] = (uint32_t) rs1_value % rs2_value;
}

emu_result_t rv64_multiplication_emulate(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    uint8_t rs2 = 0;
    uint8_t rs1 = 0;
    uint8_t rd = 0;

    rv64_decode_register_register(raw_instruction, &rs2, &rs1, &rd);

    switch(tag) {
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
