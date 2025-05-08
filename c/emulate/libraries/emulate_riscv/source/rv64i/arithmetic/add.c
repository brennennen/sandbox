
#include <stdio.h>
#include <stdint.h>

#include "rv64i/emulate_rv64i.h"

#include "rv64i/instructions/arithmetic/add.h"

/*
addi a1, a0, 5

00530293
00000000 01010011 00000010 10010011
*/

static emu_result_t read_add_immediate(
    uint32_t raw_instruction,
    int16_t* imm12,
    uint8_t* rs1,
    uint8_t* rd
) {
    *imm12 = (int16_t)((raw_instruction >> 20) & 0x0FFF);
    *rs1 = (raw_instruction >> 15) & 0b11111;
    *rd = (raw_instruction >> 7) & 0b11111;
}

emu_result_t emu_rv64i_decode_add_immediate(
    emulator_rv64i_t* emulator,
    uint32_t raw_instruction,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    int16_t imm12 = 0;
    uint8_t rs1 = 0;
    uint8_t rd = 0;

    emu_result_t res = read_add_immediate(raw_instruction, &imm12, &rs1, &rd);

    printf("imm12: %d\n", imm12);
    char* rs1_name = emu_rv64i_map_register_name(rs1);
    char* rd_name = emu_rv64i_map_register_name(rd);

    int written = snprintf(buffer + *index, buffer_size - *index,
        "addi %s, %s, %d", rd_name, rs1_name, imm12);
    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
    return(ER_SUCCESS);
}
