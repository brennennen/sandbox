
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
