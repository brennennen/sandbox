
#include <stdint.h>

#include "logger.h"

#include "rv64/rv64_decode.h"
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"
#include "rv64/rv64_decode.h"

#include "rv64/modules/rv64i_base_integer.h"

#include "rv64/disassemble/rv64a_atomic_disassemble.h"

/*
 * MARK: RV64A Main
 */

emu_result_t rv64a_atomic_disassemble(
    rv64_disassembler_t* disassembler,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    emu_result_t result = ER_FAILURE;
    uint8_t aquire = 0;
    uint8_t release = 0;
    uint8_t rs2 = 0;
    uint8_t rs1 = 0;
    uint8_t rd = 0;
    rv64a_decode_atomic(raw_instruction, &aquire, &release, &rs2, &rs1, &rd);

    printf("rv64a_atomic_disassemble: aquire: %d, release: %d, rs2: %d, rs1: %d, rd: %d\n",
        aquire, release, rs2, rs1, rd);

    char* rs1_name = rv64_map_register_name(rs1);
    char* rs2_name = rv64_map_register_name(rs2);
    char* rd_name = rv64_map_register_name(rd);
    char* tag_name = rv64_instruction_tag_mnemonic[tag];

    int written = snprintf(buffer + *index, buffer_size - *index,
        "%s %s, %s, (%s)", tag_name, rd_name, rs2_name, rs1_name);

    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
    return(ER_SUCCESS);
}
