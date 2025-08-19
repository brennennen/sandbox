
#include <stdint.h>

#include "logger.h"

#include "rv64/rv64_decode.h"
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"
#include "rv64/rv64_decode.h"

#include "rv64/instructions/rv64i_base_integer.h"

#include "rv64/disassemble/rv64a_atomic_disassemble.h"

/*
 * MARK: RV64A Main
 */

emu_result_t rv64a_atomic_disassemble(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    emu_result_t result = ER_FAILURE;
    // looks like all atomic instructions have the same decode format, probably don't
    // need a switch case

    uint8_t aq = 0;
    uint8_t rl = 0;
    uint8_t rs2 = 0;
    uint8_t rs1 = 0;
    uint8_t rd = 0;
    rv64a_decode_atomic(raw_instruction, &aq, &rl, &rs2, &rs1, &rd);

    printf("rv64a_atomic_disassemble: aq: %d, rl: %d, rs2: %d, rs1: %d, rd: %d\n",
        aq, rl, rs2, rs1, rd);

    // amoswap.w t0, t1, (t2)\n
    // rv64a_atomic_disassemble: aq: 0, rl: 0, rs2: 0, rs1: 10, rd: 0
    // test fails as: amoswap.w zero, a0, (zero)
    // TODO: why is t0 and t2 being encoded as x0?
    char* rs1_name = rv64_map_register_name(rs1);
    char* rs2_name = rv64_map_register_name(rs2);
    char* rd_name = rv64_map_register_name(rd);
    char* tag_name = rv64_instruction_tag_mnemonic[tag];

    int written = snprintf(buffer + *index, buffer_size - *index,
        "%s %s, %s, (%s)", tag_name, rd_name, rs1_name, rs2_name);

    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
    return(ER_SUCCESS);
}
