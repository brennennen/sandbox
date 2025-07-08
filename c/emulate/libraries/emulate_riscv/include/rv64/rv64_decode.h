
#ifndef RV64_DECODE_H
#define RV64_DECODE_H

#include <stdint.h>

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

#endif // RV64I_DECODE_INSTRUCTION_H
