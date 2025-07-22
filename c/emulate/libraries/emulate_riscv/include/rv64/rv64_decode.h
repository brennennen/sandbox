
#ifndef RV64_DECODE_H
#define RV64_DECODE_H

#include <stdint.h>

/**
 * Decodes the "U-type" instruction format.
 */
static inline void rv64_decode_upper_immediate(
    uint32_t raw_instruction,
    int32_t* imm20,
    uint8_t* rd
) {
    *imm20 = (raw_instruction >> 12) & 0xFFFFF;
    *rd = (raw_instruction >> 7) & 0b11111;
}

/**
 * Decode "J-Type" instruction format (I think jal is the only one?).
 * @see 2.5.1 Unconditional Jumps (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_unconditional_jumps)
 * The offset is ~21 bits, spread out in a way to make hardware easier to process them (it's
 * also sign extended). See the spec for the exact bitmapping. Only even addresses are indexable,
 * so the least significant bit is left out.
 */
static inline void rv64_decode_jal(
    uint32_t raw_instruction,
    int32_t* offset,
    uint8_t* rd
) {
    uint8_t sign_bit = (raw_instruction >> 31) & 0b1;
    uint16_t imm10 = (raw_instruction >> 21) & 0b1111111111;
    uint8_t imm1 = (raw_instruction >> 20) & 0b1;
    uint16_t imm8 = (raw_instruction >> 12) & 0b11111111;
    *offset = (sign_bit << 20) | (imm1 << 12) | (imm8 << 11) | (imm10 << 1);
    // Sign extension trick. Shift the 21 bit immediate into the most significant bits, the sign
    // bit being in the most significant place. Then arithmetic right shift back the bits to their
    // correct place to extend/replicate the sign bit to all leading bits.
    *offset = (*offset << (32 - 21)) >> (32 - 21);
    *rd = (raw_instruction >> 7) & 0b11111;
}

/**
 * Decode "B-Type" instruction format (branch instructions).
 * @see RISC-V Unprivileged ISA Manual, Section 2.5.2 Conditional Branches (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_conditional_branches)
 * Similar to J-Type, the offset is ~13 bits, spread out in a way to make hardware easier to
 * process them (also sign extended). See spec for exact bitmapping. Only even addresses are indexable,
 * so the least significant bit is left out.
 */
static inline void rv64_decode_branch(
    uint32_t raw_instruction,
    int32_t* offset,
    uint8_t* rs1,
    uint8_t* rs2
) {
    uint8_t sign_bit = (raw_instruction >> 31) & 0b1;
    uint8_t imm6 = (raw_instruction >> 25) & 0b111111;
    uint8_t imm4 = (raw_instruction >> 8) & 0b1111;
    uint8_t imm1 = (raw_instruction >> 7) & 0b1;
    *offset = (sign_bit << 12) | (imm1 << 11) | (imm6 << 5) | (imm4 << 1);
    *offset = (*offset << (32 - 13)) >> (32 - 13); // Sign extend
    *rs1 = (raw_instruction >> 15) & 0b11111;
    *rs2 = (raw_instruction >> 20) & 0b11111;
}

/**
 * Decodes the "I-type" instruction format.
 */
static inline void rv64_decode_register_immediate(
    uint32_t raw_instruction,
    int16_t* imm12,
    uint8_t* rs1,
    uint8_t* rd
) {
    *imm12 = (int16_t)((raw_instruction >> 20) & 0x0FFF);
    *rs1 = (raw_instruction >> 15) & 0b11111;
    *rd = (raw_instruction >> 7) & 0b11111;
}

/**
 * Decodes the "R-type" instruction format.
 */
static inline void rv64_decode_register_register(
    uint32_t raw_instruction,
    uint8_t* rs2,
    uint8_t* rs1,
    uint8_t* rd
) {
    *rs2 = (raw_instruction >> 20) & 0b11111;
    *rs1 = (raw_instruction >> 15) & 0b11111;
    *rd = (raw_instruction >> 7) & 0b11111;
}

/**
 * MARK: RV64V
 */

/**
 * @see 30.6 Configuration-Setting Instructions (vsetvli/vsetivli/vsetvl) (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#sec-vector-config)
 */
static inline void rv64v_decode_vsetvli(
    uint32_t raw_instruction,
    uint8_t* rd,
    uint8_t* rs1,
    uint16_t* vtypei
) {
    *rd = (raw_instruction >> 7) & 0b11111;
    *rs1 = (raw_instruction >> 15) & 0b11111;
    *vtypei = (raw_instruction >> 20) & 0b11111111111;
}

/**
 * @see 30.6.1 vtype encoding (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_vtype_encoding)
 */
static inline void rv64v_decode_vsetvli_vtypei(
    uint16_t vtypei,
    uint8_t* vill,
    uint8_t* vma,
    uint8_t* vta,
    uint8_t* vsew,
    uint8_t* vlmul
) {
    *vill = (vtypei >> 11) & 0b1;
    // 8 - 10: reserved
    *vma = (vtypei >> 7) & 0b1;
    *vta = (vtypei >> 6) & 0b1;
    *vsew = (vtypei >> 3) & 0b111;
    *vlmul = vtypei & 0b111;
    printf("vtypei: %d, vill: %d, vma: %d, vta: %d, vsew: %d, vlmul: %d\n",
        vtypei, *vill, *vma, *vta, *vsew, *vlmul);
}

/**
 * @see 30.6 Configuration-Setting Instructions (vsetvli/vsetivli/vsetvl) (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#sec-vector-config)
 */
static inline void rv64v_decode_vsetivli(
    uint32_t raw_instruction,
    uint8_t* rd,
    uint8_t* uimm,
    uint16_t* vtypei
) {
    *rd = (raw_instruction >> 7) & 0b11111;
    *uimm = (raw_instruction >> 15) & 0b11111;
    *vtypei = (raw_instruction >> 20) & 0b111111111;
}

/**
 * @see 30.6.1 vtype encoding (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_vtype_encoding)
 */
static inline void rv64v_decode_vsetivli_vtypei(
    uint16_t vtypei,
    uint8_t* vill,
    uint8_t* vma,
    uint8_t* vta,
    uint8_t* vsew,
    uint8_t* vlmul
) {
    *vill = (vtypei >> 9) & 0b1;
    // 8: reserved
    *vma = (vtypei >> 7) & 0b1;
    *vta = (vtypei >> 6) & 0b1;
    *vsew = (vtypei >> 3) & 0b111;
    *vlmul = vtypei & 0b111;
    printf("vtypei: %d, vill: %d, vma: %d, vta: %d, vsew: %d, vlmul: %d\n",
        vtypei, *vill, *vma, *vta, *vsew, *vlmul);
}

/**
 * @see 30.6 Configuration-Setting Instructions (vsetvli/vsetivli/vsetvl) (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#sec-vector-config)
 */
static inline void rv64v_decode_vsetvl(
    uint32_t raw_instruction,
    uint8_t* rd,
    uint8_t* rs1,
    uint8_t* rs2
) {
    *rd = (raw_instruction >> 7) & 0b11111;
    *rs1 = (raw_instruction >> 15) & 0b11111;
    *rs2 = (raw_instruction >> 20) & 0b11111;
}

static inline void rv64v_decode_opivv(
    uint32_t raw_instruction,
    uint8_t* vs1,
    uint8_t* vs2,
    uint8_t* vd,
    uint8_t* vm
) {
    *vs1 = (raw_instruction >> 15) & 0b11111;
    *vs2 = (raw_instruction >> 20) & 0b11111;
    *vd = (raw_instruction >> 7) & 0b11111;
    *vm = (raw_instruction >> 25) & 0b1;
}

#endif // RV64I_DECODE_INSTRUCTION_H
