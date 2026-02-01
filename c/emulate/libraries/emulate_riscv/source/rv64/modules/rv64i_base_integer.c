
#include <stdint.h>

#include "logger.h"

#include "rv64/rv64_decode.h"
#include "rv64/rv64_instructions.h"

#include "rv64/modules/rv64i_base_integer.h"

/*
 * MARK: U-Type
 */

/**
 * lui - Load Upper Immediate.
 * `lui rd, imm`
 * Places the 32-bit U-immediate into register rd, filling the lowest 12 bits
 * with zeros. The 32-bit result is sign-extended to 64 bits.
 * @see
 * https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_integer_register_immediate_instructions_2
 */
static inline void rv64i_lui(rv64_hart_t* hart, int32_t imm20, uint8_t rd) {
    // sign-extension for imm20 done in decode
    hart->registers[rd] = imm20 << 12;
}

/**
 * auipc - Add Upper Immediate to `pc`.
 * `auipc rd, imm`
 * Used to build pc-relative addresses and uses the "U-Type" format. AUIPC forms
 * a 32 bit offset from the U-immediate, filling in the lowest 12 bits with zeros,
 * adds this offset to the address of the AUIPC instruction, then places the result
 * in register rd.
 * @see
 * https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_integer_register_immediate_instructions_2
 */
static inline void rv64i_auipc(rv64_hart_t* hart, int32_t imm20, uint8_t rd) {
    // pc is incremented when the instruction is originally read, so need to decrement here.
    // sign-extension for imm20 done in decode
    hart->registers[rd] = (imm20 << 12) + hart->pc - 4;
}

static emu_result_t rv64i_emulate_upper_immediate(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    int32_t imm20 = 0;
    uint8_t rd = 0;

    rv64_decode_upper_immediate(raw_instruction, &imm20, &rd);

    switch (tag) {
        case I_RV64I_LUI: {
            rv64i_lui(hart, imm20, rd);
            break;
        }
        case I_RV64I_AUIPC: {
            rv64i_auipc(hart, imm20, rd);
            break;
        }
        default: {
            LOG(LOG_ERROR, "rv64i_emulate_upper_immediate: instruction not implemented");
            return (ER_FAILURE);
        }
    }
    return (ER_SUCCESS);
}

/*
 * MARK: J-Type
 */

/**
 * jal - Jump and Link
 * `jal rd, offset`
 * `jal rd, <label>`
 * `j label`
 * `call label`
 * Store pc 1 instruction after jump to rd, and jumps (sets pc) to offset.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_unconditional_jumps
 */
static inline void rv64i_jal(rv64_hart_t* hart, int32_t offset, uint8_t rd) {
    if (rd != 0) {
        hart->registers[rd] = hart->pc;  // point rd to the next instruction
    }
    hart->pc = hart->pc + offset - 4;
    LOGD("%s: offset: %d (pc: %ld), rd: %d", __func__, offset, hart->pc, rd);
}

static emu_result_t rv64i_emulate_j_type(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    int32_t offset = 0;
    uint8_t rd = 0;

    rv64_decode_j_type(raw_instruction, &offset, &rd);

    switch (tag) {
        case I_RV64I_JAL: {
            rv64i_jal(hart, offset, rd);
            break;
        }
        default: {
            LOG(LOG_ERROR, "rv64i_emulate_j_type: instruction not implemented");
            return (ER_FAILURE);
        }
    }
    return (ER_SUCCESS);
}

/**
 * jalr - Jump and Link Register
 * `jalr rd, rs1, offset`
 * `jr rs1, offset`
 * `ret`
 * Store pc 1 instruction after jump to rd, and jumps (sets pc) to rs1 + offset
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_unconditional_jumps
 */
static inline void rv64i_jalr(rv64_hart_t* hart, int32_t imm12, uint8_t rs1, uint8_t rd) {
    if (rd != 0) {
        hart->registers[rd] = hart->pc;  // point rd to the next instruction
    }
    LOGD("%s: imm12: %d, rs1: %ld (%d), rd: %d", __func__, imm12, hart->registers[rs1], rs1, rd);
    // TODO: does imm12 need to be left shifted 1 bit?
    hart->pc = hart->registers[rs1] + imm12;
}

/*
 * MARK: B-Type
 */

/**
 * beq - Branch if EQual.
 * `beq rs1, rs2, <label>`
 * `beq rs1, rs2, . + <jump offset>`
 * `beqz rs1, <label>`
 * If rs1 and rs2 are equal, add the offset to pc (branch out of the mainline
 * execution flow).
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_conditional_branches
 */
static inline void rv64i_beq(rv64_hart_t* hart, int32_t offset, uint8_t rs1, uint8_t rs2) {
    LOGD(
        "%s: offset: %d, rs1: %ld, rs2: %ld", __func__, offset, hart->registers[rs1],
        hart->registers[rs2]
    );
    if (hart->registers[rs1] == hart->registers[rs2]) {
        hart->pc = hart->pc + offset - 4;
    }
}

/**
 * bne - Branch Not Equal.
 * `bne rs1, rs2, <label>`
 * `bne rs1, rs2, . + <jump offset>`
 * `bnez rs1, <label>`
 * If rs1 and rs2 are not equal, add the offset to pc (branch out of the mainline
 * execution flow).
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_conditional_branches
 */
static inline void rv64i_bne(rv64_hart_t* hart, int32_t offset, uint8_t rs1, uint8_t rs2) {
    LOGD(
        "%s: offset: %d, rs1: %ld, rs2: %ld", __func__, offset, hart->registers[rs1],
        hart->registers[rs2]
    );
    if (hart->registers[rs1] != hart->registers[rs2]) {
        hart->pc = hart->pc + offset - 4;
    }
}

static inline void rv64i_blt(rv64_hart_t* hart, int32_t offset, uint8_t rs1, uint8_t rs2) {
    LOGD(
        "%s: offset: %d, rs1: %ld, rs2: %ld", __func__, offset, hart->registers[rs1],
        hart->registers[rs2]
    );
    if ((int64_t)hart->registers[rs1] < (int64_t)hart->registers[rs2]) {
        hart->pc = hart->pc + offset - 4;
    }
}

static inline void rv64i_bge(rv64_hart_t* hart, int32_t offset, uint8_t rs1, uint8_t rs2) {
    LOGD(
        "%s: offset: %d, rs1: %ld, rs2: %ld", __func__, offset, hart->registers[rs1],
        hart->registers[rs2]
    );
    if ((int64_t)hart->registers[rs1] >= (int64_t)hart->registers[rs2]) {
        hart->pc = hart->pc + offset - 4;
    }
}

static inline void rv64i_bltu(rv64_hart_t* hart, int32_t offset, uint8_t rs1, uint8_t rs2) {
    LOGD(
        "%s: offset: %d, rs1: %ld, rs2: %ld", __func__, offset, hart->registers[rs1],
        hart->registers[rs2]
    );
    if (hart->registers[rs1] < hart->registers[rs2]) {
        hart->pc = hart->pc + offset - 4;
    }
}

static inline void rv64i_bgeu(rv64_hart_t* hart, int32_t offset, uint8_t rs1, uint8_t rs2) {
    LOGD(
        "%s: offset: %d, rs1: %ld, rs2: %ld", __func__, offset, hart->registers[rs1],
        hart->registers[rs2]
    );
    if (hart->registers[rs1] >= hart->registers[rs2]) {
        hart->pc = hart->pc + offset - 4;
    }
}

static emu_result_t rv64i_emulate_b_type(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    int32_t offset = 0;
    uint8_t rs1 = 0;
    uint8_t rs2 = 0;

    rv64_decode_branch(raw_instruction, &offset, &rs1, &rs2);

    switch (tag) {
        case I_RV64I_BEQ: {
            rv64i_beq(hart, offset, rs1, rs2);
            break;
        }
        case I_RV64I_BNE: {
            rv64i_bne(hart, offset, rs1, rs2);
            break;
        }
        case I_RV64I_BLT: {
            rv64i_blt(hart, offset, rs1, rs2);
            break;
        }
        case I_RV64I_BGE: {
            rv64i_bge(hart, offset, rs1, rs2);
            break;
        }
        case I_RV64I_BLTU: {
            rv64i_bltu(hart, offset, rs1, rs2);
            break;
        }
        case I_RV64I_BGEU: {
            rv64i_bgeu(hart, offset, rs1, rs2);
            break;
        }
        default: {
            LOG(LOG_ERROR, "rv64i_emulate_b_type: instruction not implemented");
            return (ER_FAILURE);
        }
    }
    return (ER_SUCCESS);
}

/*
 * MARK: S-Type
 */

static inline void rv64i_sb(rv64_hart_t* hart, int16_t imm12, uint8_t rs1, uint8_t rs2) {
    uint64_t address = hart->registers[rs1] + imm12;
    hart->shared_system->memory[address] = hart->registers[rs2] & 0xFF;
}

static inline void rv64i_sh(rv64_hart_t* hart, int16_t imm12, uint8_t rs1, uint8_t rs2) {
    uint64_t address = hart->registers[rs1] + imm12;
    // todo: just use memcpy? uses host machine endieness?
    hart->shared_system->memory[address] = (uint8_t)(hart->registers[rs2] & 0xFF);
    hart->shared_system->memory[address + 1] = (uint8_t)((hart->registers[rs2] >> 8) & 0xFF);
}

static inline void rv64i_sw(rv64_hart_t* hart, int16_t imm12, uint8_t rs1, uint8_t rs2) {
    uint64_t address = hart->registers[rs1] + imm12;
    hart->shared_system->memory[address] = (uint8_t)(hart->registers[rs2] & 0xFF);
    hart->shared_system->memory[address + 1] = (uint8_t)((hart->registers[rs2] >> 8) & 0xFF);
    hart->shared_system->memory[address + 2] = (uint8_t)((hart->registers[rs2] >> 16) & 0xFF);
    hart->shared_system->memory[address + 3] = (uint8_t)((hart->registers[rs2] >> 24) & 0xFF);
}

/**
 * sd - Store Double-word
 * `sd rs2, <offset>(rs1)`
 * Stores 8 bytes from rs2 to memory.
 * RV64I Additional Instruction (Not in RV32I)
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_load_and_store_instructions
 */
static inline void rv64i_sd(rv64_hart_t* hart, int16_t imm12, uint8_t rs1, uint8_t rs2) {
    uint64_t address = hart->registers[rs1] + imm12;
    hart->shared_system->memory[address] = (uint8_t)(hart->registers[rs2] & 0xFF);
    hart->shared_system->memory[address + 1] = (uint8_t)((hart->registers[rs2] >> 8) & 0xFF);
    hart->shared_system->memory[address + 2] = (uint8_t)((hart->registers[rs2] >> 16) & 0xFF);
    hart->shared_system->memory[address + 3] = (uint8_t)((hart->registers[rs2] >> 24) & 0xFF);
    hart->shared_system->memory[address + 4] = (uint8_t)((hart->registers[rs2] >> 32) & 0xFF);
    hart->shared_system->memory[address + 5] = (uint8_t)((hart->registers[rs2] >> 40) & 0xFF);
    hart->shared_system->memory[address + 6] = (uint8_t)((hart->registers[rs2] >> 48) & 0xFF);
    hart->shared_system->memory[address + 7] = (uint8_t)((hart->registers[rs2] >> 56) & 0xFF);
}

static emu_result_t rv64i_emulate_s_type(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    uint16_t offset = 0;
    uint8_t rs1 = 0;
    uint8_t rs2 = 0;

    rv64_decode_store(raw_instruction, &offset, &rs1, &rs2);

    switch (tag) {
        case I_RV64I_SB: {
            rv64i_sb(hart, offset, rs1, rs2);
            break;
        }
        case I_RV64I_SH: {
            rv64i_sh(hart, offset, rs1, rs2);
            break;
        }
        case I_RV64I_SW: {
            rv64i_sw(hart, offset, rs1, rs2);
            break;
        }
        case I_RV64I_SD: {
            rv64i_sd(hart, offset, rs1, rs2);
            break;
        }
        default: {
            LOG(LOG_ERROR, "%s: instruction not implemented", __func__);
            return (ER_FAILURE);
        }
    }
    return (ER_SUCCESS);
}

/*
 * MARK: I-Type
 */

/**
 * lb - Load Byte
 * `lb rd, <offset>(rs1)`
 * Loads 1 byte from memory as a signed value, sign-extending to 64 bits, and
 * stores it in rd.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_load_and_store_instructions
 */
static inline void rv64i_lb(rv64_hart_t* hart, int16_t imm12, uint8_t rs1, uint8_t rd) {
    uint64_t address = hart->registers[rs1] + imm12;
    // cast provides sign extension
    hart->registers[rd] = (int8_t)hart->shared_system->memory[address];
}

/**
 * lh - Load Half-word
 * `lh rd, <offset>(rs1)`
 * Loads 2 bytes from memory as a signed value, sign-extending to 64 bits, and
 * stores it in rd.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_load_and_store_instructions
 */
static inline void rv64i_lh(rv64_hart_t* hart, int16_t imm12, uint8_t rs1, uint8_t rd) {
    uint64_t address = hart->registers[rs1] + imm12;
    uint16_t halfword = (uint16_t)hart->shared_system->memory[address]
                        | ((uint16_t)hart->shared_system->memory[address + 1] << 8);
    // cast provides sign extension
    hart->registers[rd] = (int16_t)halfword;
}

/**
 * lw - Load Word
 * `lw rd, <offset>(rs1)`
 * Loads 4 bytes from memory as a signed value, sign-extending to 64 bits, and
 * stores it in rd.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_load_and_store_instructions
 */
static inline void rv64i_lw(rv64_hart_t* hart, int16_t imm12, uint8_t rs1, uint8_t rd) {
    // TODO: sign extend
    uint64_t address = hart->registers[rs1] + imm12;
    uint32_t word = (uint32_t)hart->shared_system->memory[address]
                    | ((uint32_t)hart->shared_system->memory[address + 1] << 24)
                    | ((uint32_t)hart->shared_system->memory[address + 2] << 16)
                    | ((uint32_t)hart->shared_system->memory[address + 3] << 8);
    // cast provides sign extension
    hart->registers[rd] = (int32_t)word;
}

/**
 * lbu - Load Byte Unsigned
 * `lbu rd, <offset>(rs1)`
 * Loads 1 byte from memory as an unsigned value, zero-extending to 64 bits, and
 * stores it in rd.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_load_and_store_instructions
 */
static inline void rv64i_lbu(rv64_hart_t* hart, int16_t imm12, uint8_t rs1, uint8_t rd) {
    uint64_t address = hart->registers[rs1] + imm12;
    hart->registers[rd] = hart->shared_system->memory[address];
}

/**
 * lhu - Load Half-word Unsigned
 * `lhu rd, <offset>(rs1)`
 * Loads 2 bytes from memory as an unsigned value, zero-extending to 64 bits, and
 * stores it in rd.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_load_and_store_instructions
 */
static inline void rv64i_lhu(rv64_hart_t* hart, int16_t imm12, uint8_t rs1, uint8_t rd) {
    uint64_t address = hart->registers[rs1] + imm12;
    hart->registers[rd] = (uint16_t)hart->shared_system->memory[address]
                          | ((uint16_t)hart->shared_system->memory[address + 1] << 8);
}

/**
 * lwu - Load Word Unsigned
 * `lwu rd, <offset>(rs1)`
 * Loads 4 bytes from memory as an unsigned value, zero-extending to 64
 * bits, and stores it in rd.
 * RV64I Additional Instruction (Not in RV32I)
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_load_and_store_instructions
 */
static inline void rv64i_lwu(rv64_hart_t* hart, int16_t imm12, uint8_t rs1, uint8_t rd) {
    uint64_t address = hart->registers[rs1] + imm12;
    uint32_t word = (uint32_t)hart->shared_system->memory[address]
                    | ((uint32_t)hart->shared_system->memory[address + 1] << 24)
                    | ((uint32_t)hart->shared_system->memory[address + 2] << 16)
                    | ((uint32_t)hart->shared_system->memory[address + 3] << 8);
    hart->registers[rd] = word;
}

/**
 * ld - Load Double-word
 * `ld rd, <offset>(rs1)`
 * Loads 8 bytes from memory as a signed value and stores it in rd.
 * RV64I Additional Instruction (Not in RV32I)
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_load_and_store_instructions
 */
static inline void rv64i_ld(rv64_hart_t* hart, int16_t imm12, uint8_t rs1, uint8_t rd) {
    uint64_t address = hart->registers[rs1] + imm12;
    // todo: why not just memcpy?
    uint64_t double_word = (uint64_t)hart->shared_system->memory[address]
                           | ((uint64_t)hart->shared_system->memory[address + 1] << 56)
                           | ((uint64_t)hart->shared_system->memory[address + 3] << 48)
                           | ((uint64_t)hart->shared_system->memory[address + 4] << 40)
                           | ((uint64_t)hart->shared_system->memory[address + 5] << 32)
                           | ((uint64_t)hart->shared_system->memory[address + 6] << 24)
                           | ((uint64_t)hart->shared_system->memory[address + 7] << 16)
                           | ((uint64_t)hart->shared_system->memory[address + 8] << 8);
    hart->registers[rd] = double_word;
}

/**
 * Adds sign extended 12-bit immediate to register rs1 and sign extends as a 32 bit result.
 * RV64I Additional Instruction (Not in RV32I)
 * @see
 * https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_integer_register_immediate_instructions_2
 */
static inline void rv64i_addiw(rv64_hart_t* hart, int16_t imm12, uint8_t rs1, uint8_t rd) {
    hart->registers[rd] = (int32_t)(hart->registers[rs1] + imm12);
}

/**
 * NOTE: NOP is encoded as `ADDI x0, x0, 0`.
 */
static inline void rv64_addi(rv64_hart_t* hart, int16_t imm12, uint8_t rs1, uint8_t rd) {
    hart->registers[rd] = hart->registers[rs1] + imm12;
}

/**
 * "Set Less Than Immediate" - Set rd to 1 if rs1 is less than the provided immediate when
 * both numbers are treated as signed.
 */
static inline void rv64_slti(rv64_hart_t* hart, int16_t imm12, uint8_t rs1, uint8_t rd) {
    if ((int64_t)hart->registers[rs1] < (int64_t)imm12) {
        hart->registers[rd] = 1;
    } else {
        hart->registers[rd] = 0;
    }
    LOGD(
        "%s: rs1: %d, imm: %d, rd: %d", __func__, hart->registers[rs1], imm12, hart->registers[rd]
    );
}

/**
 * "Set Less Than Immediate Unsigned" - Set rd to 1 if rs1 is less than the immedate when
 * both numbers are treated as unsigned.
 */
static inline void rv64_sltiu(rv64_hart_t* hart, int16_t imm12, uint8_t rs1, uint8_t rd) {
    if ((uint64_t)hart->registers[rs1] < (uint16_t)imm12) {
        hart->registers[rd] = 1;
    } else {
        hart->registers[rd] = 0;
    }
    LOGD(
        "%s: rs1: %d, imm: %d, rd: %d", __func__, hart->registers[rs1], imm12, hart->registers[rd]
    );
}

static inline void rv64_xori(rv64_hart_t* hart, int16_t imm12, uint8_t rs1, uint8_t rd) {
    hart->registers[rd] = hart->registers[rs1] ^ imm12;
}

/**
 * "logical/bitwise OR" - Both numbers sign extended.
 */
static inline void rv64_ori(rv64_hart_t* hart, int16_t imm12, uint8_t rs1, uint8_t rd) {
    hart->registers[rd] = hart->registers[rs1] | imm12;
}

/**
 * "logical/bitwise AND" - Both numbers sign extended.
 */
static inline void rv64_andi(rv64_hart_t* hart, int16_t imm12, uint8_t rs1, uint8_t rd) {
    hart->registers[rd] = hart->registers[rs1] & imm12;
}

static emu_result_t rv64_emulate_register_immediate(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    int16_t imm12 = 0;
    uint8_t rs1 = 0;
    uint8_t rd = 0;

    rv64_decode_register_immediate(raw_instruction, &imm12, &rs1, &rd);

    switch (tag) {
        case I_RV64I_JALR: {
            rv64i_jalr(hart, imm12, rs1, rd);
            break;
        }
        case I_RV64I_LB: {
            rv64i_lb(hart, imm12, rs1, rd);
            break;
        }
        case I_RV64I_LH: {
            rv64i_lh(hart, imm12, rs1, rd);
            break;
        }
        case I_RV64I_LW: {
            rv64i_lw(hart, imm12, rs1, rd);
            break;
        }
        case I_RV64I_LBU: {
            rv64i_lbu(hart, imm12, rs1, rd);
            break;
        }
        case I_RV64I_LHU: {
            rv64i_lhu(hart, imm12, rs1, rd);
            break;
        }
        case I_RV64I_ADDI: {
            rv64_addi(hart, imm12, rs1, rd);
            break;
        }
        case I_RV64I_SLTI: {
            rv64_slti(hart, imm12, rs1, rd);
            break;
        }
        case I_RV64I_SLTIU: {
            rv64_sltiu(hart, imm12, rs1, rd);
            break;
        }
        case I_RV64I_XORI: {
            rv64_xori(hart, imm12, rs1, rd);
            break;
        }
        case I_RV64I_ORI: {
            rv64_ori(hart, imm12, rs1, rd);
            break;
        }
        case I_RV64I_ANDI: {
            rv64_andi(hart, imm12, rs1, rd);
            break;
        }
        case I_RV64I_LWU: {
            rv64i_lwu(hart, imm12, rs1, rd);
            break;
        }
        case I_RV64I_LD: {
            rv64i_ld(hart, imm12, rs1, rd);
            break;
        }
        case I_RV64I_ADDIW: {
            rv64i_addiw(hart, imm12, rs1, rd);
            break;
        }
        default: {
            LOG(LOG_ERROR, "rv64i_emulate_register_immediate: instruction not implemented");
            return (ER_FAILURE);
        }
    }
    return (ER_SUCCESS);
}

/**
 * SLLI - Shift Logical Left with Immediate.
 * Zeros are shifted into the lower bits.
 * (Section 2.4.1. Integer Register-Immediate Instructions)
 */
static inline void rv64_slli(rv64_hart_t* hart, int16_t imm12, uint8_t rs1, uint8_t rd) {
    hart->registers[rd] = hart->registers[rs1] << imm12;
}

/**
 * SRLI - Shift Right Logical with Immediate.
 * Zeros are shifted into the upper bits.
 * (Section 2.4.1. Integer Register-Immediate Instructions)
 */
static inline void rv64_srli(rv64_hart_t* hart, int16_t imm12, uint8_t rs1, uint8_t rd) {
    hart->registers[rd] = hart->registers[rs1] >> ((uint16_t)imm12);
}

/**
 * SRAI - Shift Right Arithmetic with Immediate.
 * The original sign bit is copied into the vacated upper bits.
 * (Section 2.4.1. Integer Register-Immediate Instructions)
 */
static inline void rv64_srai(rv64_hart_t* hart, int16_t imm12, uint8_t rs1, uint8_t rd) {
    hart->registers[rd] = ((int64_t)hart->registers[rs1]) >> imm12;
}

/**
 * slli - Shift Logical Left with Immediate.
 * `slli rd, rs1, imm5`
 * Zeros are shifted into the lower bits.
 * @see
 * https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_integer_register_immediate_instructions
 */
static inline void rv64i_slli(rv64_hart_t* hart, uint8_t imm5, uint8_t rs1, uint8_t rd) {
    hart->registers[rd] = hart->registers[rs1] << imm5;
}

/**
 * srli - Shift Logical Right with Immediate.
 * `srli rd, rs1, imm5`
 * Zeros are shifted into the lower bits.
 * @see
 * https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_integer_register_immediate_instructions
 */
static inline void rv64i_srli(rv64_hart_t* hart, uint8_t imm5, uint8_t rs1, uint8_t rd) {
    hart->registers[rd] = hart->registers[rs1] >> ((uint8_t)imm5);
}

/**
 * srai - Shift Right Arithmetic with Immediate.
 * `srai rd, rs1, imm5`
 * The original sign bit is copied into the vacated upper bits (sign extension).
 * @see
 * https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_integer_register_immediate_instructions
 */
static inline void rv64i_srai(rv64_hart_t* hart, uint8_t imm5, uint8_t rs1, uint8_t rd) {
    hart->registers[rd] = ((int64_t)hart->registers[rs1]) >> imm5;
}

static emu_result_t rv64i_emulate_shift_immediate(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    uint8_t imm5 = 0;
    uint8_t rs1 = 0;
    uint8_t rd = 0;

    rv64_decode_shift_immediate(raw_instruction, &imm5, &rs1, &rd);

    switch (tag) {
        case I_RV64I_SLLI: {
            rv64i_slli(hart, imm5, rs1, rd);
            break;
        }
        case I_RV64I_SRLI: {
            rv64i_srli(hart, imm5, rs1, rd);
            break;
        }
        case I_RV64I_SRAI: {
            rv64i_srai(hart, imm5, rs1, rd);
            break;
        }
        default: {
            LOG(LOG_ERROR, "%s: instruction not implemented", __func__);
            return (ER_FAILURE);
        }
    }
    return (ER_SUCCESS);
}

/*
 * MARK: R-Type
 */

static inline void rv64i_add(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    hart->registers[rd] = hart->registers[rs1] + hart->registers[rs2];
}

static inline void rv64i_sub(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    hart->registers[rd] = hart->registers[rs1] - hart->registers[rs2];
}

static inline void rv64i_sll(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    hart->registers[rd] = hart->registers[rs1] << hart->registers[rs2];
}

/**
 * "Set Less Than" - Set rd to 1 if rs1 is less than rs2 when both registers are treated
 * as signed numbers.
 */
static inline void rv64i_slt(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    if (hart->registers[rs1] < hart->registers[rs2]) {
        hart->registers[rd] = 1;
    } else {
        hart->registers[rd] = 0;
    }
}

/**
 * "Set Less Than Unsigned" - Set rd to 1 if rs1 is less than rs2 when both registers
 * are treated as unsigned numbers.
 */
static inline void rv64i_sltu(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    if ((uint64_t)hart->registers[rs1] < (uint64_t)hart->registers[rs2]) {
        hart->registers[rd] = 1;
    } else {
        hart->registers[rd] = 0;
    }
}

static inline void rv64i_xor(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    hart->registers[rd] = hart->registers[rs1] ^ hart->registers[rs2];
}

// c shift right is logical when unsigned, arithmetic when signed (sign extension)
static inline void rv64i_srl(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    hart->registers[rd] = hart->registers[rs1] >> hart->registers[rs2];
}

static inline void rv64i_sra(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    hart->registers[rd] = ((int64_t)hart->registers[rs1]) >> ((int64_t)hart->registers[rs2]);
}

static inline void rv64i_or(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    hart->registers[rd] = hart->registers[rs1] | hart->registers[rs2];
}

static inline void rv64i_and(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    hart->registers[rd] = hart->registers[rs1] & hart->registers[rs2];
}

static inline void rv64i_addw(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    hart->registers[rd] = (int32_t)(hart->registers[rs1] + hart->registers[rs2]);
}

static inline void rv64i_subw(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    hart->registers[rd] = (int32_t)(hart->registers[rs1] - hart->registers[rs2]);
}

static inline void rv64i_sllw(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    uint32_t shamt = hart->registers[rs2] & 0x1F;
    hart->registers[rd] = (int32_t)(hart->registers[rs1] << shamt);
}

static inline void rv64i_srlw(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    uint32_t shamt = hart->registers[rs2] & 0x1F;
    hart->registers[rd] = (int32_t)((uint32_t)hart->registers[rs1] >> shamt);
}

static inline void rv64i_sraw(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    uint32_t shamt = hart->registers[rs2] & 0x1F;
    hart->registers[rd] = (int32_t)((int32_t)hart->registers[rs1] >> shamt);
}

static emu_result_t rv64_emulate_register_register(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    uint8_t rs2 = 0;
    uint8_t rs1 = 0;
    uint8_t rd = 0;

    rv64_decode_register_register(raw_instruction, &rs2, &rs1, &rd);

    switch (tag) {
        case I_RV64I_ADD: {
            rv64i_add(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64I_SUB: {
            rv64i_sub(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64I_SLL: {
            rv64i_sll(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64I_SLT: {
            rv64i_slt(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64I_SLTU: {
            rv64i_sltu(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64I_XOR: {
            rv64i_xor(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64I_SRL: {
            rv64i_srl(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64I_SRA: {
            rv64i_sra(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64I_OR: {
            rv64i_or(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64I_AND: {
            rv64i_and(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64I_ADDW: {
            rv64i_addw(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64I_SUBW: {
            rv64i_subw(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64I_SLLW: {
            rv64i_sllw(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64I_SRLW: {
            rv64i_srlw(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64I_SRAW: {
            rv64i_sraw(hart, rs1, rs2, rd);
            break;
        }
        default: {
            LOG(LOG_ERROR, "rv64i_emulate_register_register: instruction not implemented");
            return (ER_FAILURE);
        }
    }
    return (ER_SUCCESS);
}

/*
 * MARK: Misc
 */

static emu_result_t rv64i_fence(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    printf("%s:\n", __func__);
    // TODO: call registered callbacks? setup some defaults like prints?
    return (ER_SUCCESS);
}

static emu_result_t rv64i_fence_tso(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    printf("%s:\n", __func__);
    // TODO: call registered callbacks? setup some defaults like prints?
    return (ER_SUCCESS);
}

static emu_result_t rv64i_pause(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    printf("%s:\n", __func__);
    // TODO: call registered callbacks? setup some defaults like prints?
    return (ER_SUCCESS);
}

static emu_result_t rv64i_ecall(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    printf("%s:\n", __func__);
    // TODO: call registered callbacks? setup some defaults like prints?
    return (ER_SUCCESS);
}

static emu_result_t rv64i_ebreak(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    printf("%s:\n", __func__);
    // TODO: call registered callbacks? setup some defaults like prints?
    return (ER_SUCCESS);
}

/*
 * MARK: RV64I Main
 */

emu_result_t rv64i_base_integer_emulate(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    emu_result_t result = ER_FAILURE;
    switch (tag) {
        // Core Format "U" - "upper-immediate"
        case I_RV64I_LUI:
        case I_RV64I_AUIPC: {
            result = rv64i_emulate_upper_immediate(hart, raw_instruction, tag);
            break;
        }
        // Core Format "J" - "jump"
        case I_RV64I_JAL: {
            result = rv64i_emulate_j_type(hart, raw_instruction, tag);
            break;
        }
        case I_RV64I_JALR: {
            result = rv64_emulate_register_immediate(hart, raw_instruction, tag);
            break;
        }

        // Core Format "B" - "branch"
        case I_RV64I_BEQ:
        case I_RV64I_BNE:
        case I_RV64I_BLT:
        case I_RV64I_BGE:
        case I_RV64I_BLTU:
        case I_RV64I_BGEU: {
            result = rv64i_emulate_b_type(hart, raw_instruction, tag);
            break;
        }

        // Core Format "S" - "store"
        case I_RV64I_SB:
        case I_RV64I_SH:
        case I_RV64I_SW:
        case I_RV64I_SD: {
            result = rv64i_emulate_s_type(hart, raw_instruction, tag);
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
        case I_RV64I_LWU:
        case I_RV64I_LD:
        case I_RV64I_ADDIW: {
            result = rv64_emulate_register_immediate(hart, raw_instruction, tag);
            break;
        }
        // Special "shift immediate" format
        case I_RV64I_SLLI:
        case I_RV64I_SRLI:
        case I_RV64I_SRAI: {
            result = rv64i_emulate_shift_immediate(hart, raw_instruction, tag);
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
        case I_RV64I_AND:
        case I_RV64I_ADDW:
        case I_RV64I_SUBW:
        case I_RV64I_SLLW:
        case I_RV64I_SRLW:
        case I_RV64I_SRAW: {
            result = rv64_emulate_register_register(hart, raw_instruction, tag);
            break;
        }
        // misc
        case I_RV64I_FENCE: {
            result = rv64i_fence(hart, raw_instruction, tag);
            break;
        }
        case I_RV64I_FENCE_TSO: {
            result = rv64i_fence_tso(hart, raw_instruction, tag);
            break;
        }
        case I_RV64I_PAUSE: {
            result = rv64i_pause(hart, raw_instruction, tag);
            break;
        }
        case I_RV64I_ECALL: {
            result = rv64i_ecall(hart, raw_instruction, tag);
            break;
        }
        case I_RV64I_EBREAK: {
            result = rv64i_ebreak(hart, raw_instruction, tag);
            break;
        }
        default: {
            LOG(LOG_ERROR, "rv64i_base_integer_emulate: instruction not implemented");
            return (ER_FAILURE);
        }
    }
    return (result);
}

/*
 * MARK: Zicsr
 */

static inline void rv64i_csrrw(rv64_hart_t* hart, uint8_t csr, uint8_t rs1, uint8_t rd) {
    uint64_t rs1_temp =
        hart->registers[rs1];  // need to use a local here incase rs1 and rd are the same register.
    if (rd != 0) {
        hart->registers[rd] = rv64_get_csr_value(&hart->csrs, csr);
    }
    rv64_set_csr_value(&hart->csrs, csr, rs1_temp);
}

static inline void rv64i_csrrs(rv64_hart_t* hart, uint8_t csr, uint8_t rs1, uint8_t rd) {
    uint64_t rs1_mask =
        hart->registers[rs1];  // need to use a local here incase rs1 and rd are the same register.
    uint64_t result = rv64_get_csr_value(&hart->csrs, csr);
    if (rd != 0) {
        hart->registers[rd] = result;
    }
    if (rs1 != 0) {
        result = result ^ rs1_mask;  // rs1 mask contains bits to clear
        rv64_set_csr_value(&hart->csrs, csr, result);
    }
}

static inline void rv64i_csrrc(rv64_hart_t* hart, uint8_t csr, uint8_t rs1, uint8_t rd) {
    uint64_t rs1_mask =
        hart->registers[rs1];  // need to use a local here incase rs1 and rd are the same register.
    uint64_t result = rv64_get_csr_value(&hart->csrs, csr);
    if (rd != 0) {
        hart->registers[rd] = result;
    }
    if (rs1 != 0) {
        result = result & (~rs1_mask);  // rs1 mask contains bits to clear
        rv64_set_csr_value(&hart->csrs, csr, result);
    }
}

static emu_result_t rv64i_csr_register(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    emu_result_t result = ER_FAILURE;

    uint8_t csr = 0;
    uint8_t rs1 = 0;
    uint8_t rd = 0;

    rv64_decode_csr_register(raw_instruction, &csr, &rs1, &rd);

    switch (tag) {
        case I_RV64ZICSR_CSRRW: {
            rv64i_csrrw(hart, csr, rs1, rd);
            break;
        }
        case I_RV64ZICSR_CSRRS: {
            rv64i_csrrs(hart, csr, rs1, rd);
            break;
        }
        case I_RV64ZICSR_CSRRC: {
            rv64i_csrrc(hart, csr, rs1, rd);
            break;
        }
        default: {
            LOG(LOG_ERROR, "%s: instruction not implemented", __func__);
            return (ER_FAILURE);
        }
    }
    return (ER_SUCCESS);
}

static inline void rv64i_csrrwi(rv64_hart_t* hart, uint8_t csr, uint8_t uimm, uint8_t rd) {
    if (rd != 0) {
        hart->registers[rd] = rv64_get_csr_value(&hart->csrs, csr);
    }
    rv64_set_csr_value(&hart->csrs, csr, uimm);
}

static inline void rv64i_csrrsi(rv64_hart_t* hart, uint8_t csr, uint8_t uimm, uint8_t rd) {
    uint64_t result = rv64_get_csr_value(&hart->csrs, csr);
    if (rd != 0) {
        hart->registers[rd] = result;
    }
    if (uimm != 0) {
        result = result ^ uimm;
        rv64_set_csr_value(&hart->csrs, csr, result);
    }
}

static inline void rv64i_csrrci(rv64_hart_t* hart, uint8_t csr, uint8_t uimm, uint8_t rd) {
    uint64_t result = rv64_get_csr_value(&hart->csrs, csr);
    if (rd != 0) {
        hart->registers[rd] = result;
    }
    if (uimm != 0) {
        result = result & (~uimm);  // uimm mask contains bits to clear
        rv64_set_csr_value(&hart->csrs, csr, result);
    }
}

static emu_result_t rv64i_csr_immediate(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    printf("%s:\n", __func__);
    emu_result_t result = ER_FAILURE;

    uint8_t csr = 0;
    uint8_t uimm = 0;
    uint8_t rd = 0;

    rv64_decode_csr_immediate(raw_instruction, &csr, &uimm, &rd);

    switch (tag) {
        case I_RV64ZICSR_CSRRWI: {
            rv64i_csrrwi(hart, csr, uimm, rd);
            break;
        }
        case I_RV64ZICSR_CSRRSI: {
            rv64i_csrrsi(hart, csr, uimm, rd);
            break;
        }
        case I_RV64ZICSR_CSRRCI: {
            rv64i_csrrci(hart, csr, uimm, rd);
            break;
        }
        default: {
            LOG(LOG_ERROR, "%s: instruction not implemented", __func__);
            return (ER_FAILURE);
        }
    }
    return (ER_SUCCESS);
}

emu_result_t rv64i_zicsr_emulate(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    emu_result_t result = ER_FAILURE;
    switch (tag) {
        case I_RV64ZICSR_CSRRW:
        case I_RV64ZICSR_CSRRS:
        case I_RV64ZICSR_CSRRC: {
            result = rv64i_csr_register(hart, raw_instruction, tag);
            break;
        }
        case I_RV64ZICSR_CSRRWI:
        case I_RV64ZICSR_CSRRSI:
        case I_RV64ZICSR_CSRRCI: {
            result = rv64i_csr_immediate(hart, raw_instruction, tag);
            break;
        }
        default: {
            LOG(LOG_ERROR, "%s: instruction not implemented", __func__);
            return (ER_FAILURE);
        }
    }
    return result;
}
