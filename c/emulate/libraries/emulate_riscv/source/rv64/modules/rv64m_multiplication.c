
#include <stdint.h>
#include <assert.h>

#include "logger.h"

#include "rv64/rv64_decode.h"
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"

#include "rv64/modules/rv64m_multiplication.h"

/**
 * MUL performs a 64 bit by 64 bit multiplication of rs1 by rs2 and places the lower 64 bits
 * in the destination register.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_multiplication_operations
 */
static inline void rv64m_mul(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    hart->registers[rd] = hart->registers[rs1] * hart->registers[rs2];
}

/**
 * mulh - MULtiply High (Signed)
 * Multiplies a signed 64-bit value with a signed 64-bit value into a 128 bit result,
 * returns the upper 64 bits.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_multiplication_operations
 */
static inline void rv64m_mulh(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
#if defined(__GNUC__) || defined(__clang)
    __int128_t product = (__int128_t)hart->registers[rs1] * (__int128_t)hart->registers[rs2];
    hart->registers[rd] = (uint64_t)(product >> 64);
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
static inline void rv64m_mulhsu(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
#if defined(__GNUC__) || defined(__clang)
    // need casting for sign extension
    __int128_t product = (__int128_t)((int64_t)hart->registers[rs1] * (__uint128_t)hart->registers[rs2]);
    hart->registers[rd] = (int64_t)(product >> 64);
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
static inline void rv64m_mulhu(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
#if defined(__GNUC__) || defined(__clang)
    __uint128_t product = (__uint128_t)hart->registers[rs1] * (__uint128_t)hart->registers[rs2];
    hart->registers[rd] = (uint64_t)(product >> 64);
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
static inline void rv64m_div(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    // divide by 0
    if (hart->registers[rs2] == 0) {
        hart->registers[rd] = -1;
    }
    // signed integer overflow
    else if ( hart->registers[rs1] == 0x8000000000000000 && \
                (int64_t)(hart->registers[rs2]) == -1)
    {
        hart->registers[rd] = 0x8000000000000000;
    }
    // common case
    else {
        hart->registers[rd] = ((int64_t)hart->registers[rs1]) / ((int64_t)hart->registers[rs2]);
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
static inline void rv64m_divu(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    // divide by 0
    if (hart->registers[rs2] == 0) {
        hart->registers[rd] = 0xFFFFFFFFFFFFFFFF; // 2^64 - 1
    }
    // common case
    else {
        hart->registers[rd] = hart->registers[rs1] / hart->registers[rs2];
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
static inline void rv64m_rem(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    // divide by 0
    if (hart->registers[rs2] == 0) {
        hart->registers[rd] = hart->registers[rs1];
    }
    // signed division overflow
    else if ( hart->registers[rs1] == 0x8000000000000000 && \
                (int64_t)(hart->registers[rs2]) == -1)
    {
        hart->registers[rd] = 0;
    }
    // common case
    hart->registers[rd] = hart->registers[rs1] % hart->registers[rs2];
}

/**
 * remu - REMainder Unsigned
 * For REM, the sign of a nonzero result equals the sign of the dividend.
 * dividend = divisor * quotient + remainder
 * Edge cases (table 11):
 * * Divide by 0: res = x
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_division_operations
 */
static inline void rv64m_remu(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    // divide by 0
    if (hart->registers[rs2] == 0) {
        hart->registers[rd] = hart->registers[rs1];
    }
    // common case
    hart->registers[rd] = hart->registers[rs1] % hart->registers[rs2];
}

/**
 * mulw - MULtiply Word
 * 32 bit mul
 */
static inline void rv64m_mulw(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    int32_t rs1_value = hart->registers[rs1];
    int32_t rs2_value = hart->registers[rs2];
    hart->registers[rd] = (int32_t) rs1_value * rs2_value;
}

static inline void rv64m_divw(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    int32_t rs1_value = hart->registers[rs1];
    int32_t rs2_value = hart->registers[rs2];
    hart->registers[rd] = (int32_t) rs1_value / rs2_value;
}

static inline void rv64m_divuw(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    uint32_t rs1_value = hart->registers[rs1];
    uint32_t rs2_value = hart->registers[rs2];
    hart->registers[rd] = (uint32_t) rs1_value / rs2_value;
}

// "REMW and REMUW are RV64 instructions that provide the corresponding signed and unsigned
// remainder operations. Both REMW and REMUW always sign-extend the 32-bit result to 64 bits,
// including on a divide by zero."
static inline void rv64m_remw(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    int32_t rs1_value = hart->registers[rs1];
    int32_t rs2_value = hart->registers[rs2];
    hart->registers[rd] = (int32_t) rs1_value % rs2_value;
}

// "REMW and REMUW are RV64 instructions that provide the corresponding signed and unsigned
// remainder operations. Both REMW and REMUW always sign-extend the 32-bit result to 64 bits,
// including on a divide by zero."
static inline void rv64m_remuw(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    uint32_t rs1_value = hart->registers[rs1];
    uint32_t rs2_value = hart->registers[rs2];
    hart->registers[rd] = (uint32_t) rs1_value % rs2_value;
}

emu_result_t rv64_multiplication_emulate(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    uint8_t rs2 = 0;
    uint8_t rs1 = 0;
    uint8_t rd = 0;

    rv64_decode_register_register(raw_instruction, &rs2, &rs1, &rd);

    switch(tag) {
        case I_RV64M_MUL: {
            rv64m_mul(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64M_MULH: {
            rv64m_mulh(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64M_MULHSU: {
            rv64m_mulhsu(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64M_MULHU: {
            rv64m_mulhu(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64M_DIV: {
            rv64m_div(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64M_DIVU: {
            rv64m_divu(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64M_REM: {
            rv64m_rem(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64M_REMU: {
            rv64m_remu(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64M_MULW: {
            rv64m_mulw(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64M_DIVW: {
            rv64m_divw(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64M_DIVUW: {
            rv64m_divuw(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64M_REMW: {
            rv64m_remw(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64M_REMUW: {
            rv64m_remuw(hart, rs1, rs2, rd);
            break;
        }
        default: {
            LOG(LOG_ERROR, "%s: instruction not implemented", __func__);
            return(ER_FAILURE);
        }
    }
    return(ER_SUCCESS);
}
