
#include <string.h>

#include "shared/include/result.h"

#include "logger.h"

#include "rv64/rv64_instructions.h"
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_decode_instruction.h"

#include "rv64/instructions/rv64i_base_integer.h"
#include "rv64/instructions/rv64m_multiplication.h"


/**
 * jalr - Jump and Link Register
 * `jalr rd, rs1, offset`
 * `jr rs1, offset`
 * `ret`
 * Store pc 1 instruction after jump to rd, and jumps (sets pc) to rs1 + offset
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_unconditional_jumps
 */

 /**
  * amoswap.w
  *
  */
static inline void rv64a_amoswap_w(
    emulator_rv64_t* emulator,
    uint8_t aquire,
    uint8_t release,
    uint8_t rs2,
    uint8_t rs1,
    uint8_t rd
) {

    // todo
}






/*
 * MARK: RV64A Main
 */

emu_result_t rv64a_atomic_emulate(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    emu_result_t result = ER_FAILURE;

    uint8_t aquire = 0;
    uint8_t release = 0;
    uint8_t rs2 = 0;
    uint8_t rs1 = 0;
    uint8_t rd = 0;
    rv64a_decode_atomic(raw_instruction, &aquire, &release, &rs2, &rs1, &rd);

    printf("rv64a_atomic_emulate: aquire: %d, release: %d, rs2: %d, rs1: %d, rd: %d\n",
        aquire, release, rs2, rs1, rd);

    switch(tag) {
        case I_RV64ZAAMO_AMOSWAP_W: {
            rv64a_amoswap_w(emulator, aquire, release, rs2, rs1, rd);
        }

        // tags todo:
        case I_RV64ZALRSC_LR_W:
        case I_RV64ZALRSC_SC_W:

        case I_RV64ZAAMO_AMOADD_W:
        case I_RV64ZAAMO_AMOXOR_W:
        case I_RV64ZAAMO_AMOAND_W:
        case I_RV64ZAAMO_AMOOR_W:
        case I_RV64ZAAMO_AMOMIN_W:
        case I_RV64ZAAMO_AMOMAX_W:
        case I_RV64ZAAMO_AMOMINU_W:
        case I_RV64ZAAMO_AMOMAXU_W:
        case I_RV64ZALRSC_LR_D:
        case I_RV64ZALRSC_SC_D:
        case I_RV64ZAAMO_AMOSWAP_D:
        case I_RV64ZAAMO_AMOADD_D:
        case I_RV64ZAAMO_AMOXOR_D:
        case I_RV64ZAAMO_AMOAND_D:
        case I_RV64ZAAMO_AMOOR_D:
        case I_RV64ZAAMO_AMOMIN_D:
        case I_RV64ZAAMO_AMOMAX_D:
        case I_RV64ZAAMO_AMOMINU_D:
        case I_RV64ZAAMO_AMOMAXU_D:
        default: {
            result = ER_FAILURE;
            break;
        }
    }
    if (result != ER_SUCCESS) {
        fprintf(stderr, "Failed to parse instruction! decode_result = %s (%d)\n", emulate_result_strings[result], result);
        return(RI_FAILURE);
    }
    return(RI_CONTINUE);
}
