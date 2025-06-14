
#include <stdint.h>

#include "rv64i/rv64i_decode.h"
#include "rv64i/rv64i_emulate.h"
#include "rv64i/rv64i_instructions.h"

#include "rv64i/instructions/rv64i_register_immediate.h"

emu_result_t rv64i_disassemble_register_register(
    emulator_rv64i_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64i_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    uint8_t rs2 = 0;
    uint8_t rs1 = 0;
    uint8_t rd = 0;

    rv64i_decode_register_register(raw_instruction, &rs2, &rs1, &rd);

    char* rs1_name = emu_rv64i_map_register_name(rs1);
    char* rs2_name = emu_rv64i_map_register_name(rs2);
    char* rd_name = emu_rv64i_map_register_name(rd);
    char* tag_name = rv64i_instruction_tag_mnemonic[tag];

    int written = snprintf(buffer + *index, buffer_size - *index,
        "%s %s, %s, %s", tag_name, rd_name, rs1_name, rs2_name);

    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
    return(ER_SUCCESS);
}
