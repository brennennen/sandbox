
#include <stdio.h>
#include <stdint.h>

#include "a64/emulate_a64.h"

#include "a64/instructions/arithmetic/add.h"



static emu_result_t read_add_immediate(
    uint32_t raw_instruction,
    uint8_t* sf,
    uint8_t* op,
    uint8_t* s,
    uint8_t* sh,
    uint16_t* imm12,
    uint8_t* rn,
    uint8_t* rd
) {
    *sf = (raw_instruction >> 31);
    *op = (raw_instruction >> 30) & 0b1;
    *s = (raw_instruction >> 29) & 0b1;
    *sh = (raw_instruction >> 22) & 0b1;
    *imm12 = (raw_instruction >> 10) & 0x0FFF;
    *rn = (raw_instruction >> 5) & 0x0F;
    *rd = raw_instruction & 0x0F;
}

emu_result_t emu_a64_decode_add_immediate(
    emulator_a64_t* emulator,
    uint32_t raw_instruction,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    uint8_t sf = 0;
    uint8_t op = 0;
    uint8_t s = 0;
    uint8_t sh = 0;
    uint16_t imm12 = 0;
    uint8_t rn = 0;
    uint8_t rd = 0;
    emu_result_t res = read_add_immediate(raw_instruction, &sf, &op, &s, &sh, &imm12, &rn, &rd);
    int written = snprintf(buffer + *index,  buffer_size - *index, "add x%d, x%d, #%d", rd, rn, imm12);
    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
    return(ER_SUCCESS);
}
