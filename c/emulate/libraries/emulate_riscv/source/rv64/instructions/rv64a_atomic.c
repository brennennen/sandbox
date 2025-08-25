
#include <string.h>

#include "shared/include/result.h"

#include "logger.h"

#include "rv64/rv64_instructions.h"
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_decode.h"
#include "rv64/rv64_decode_instruction.h"

#include "rv64/instructions/rv64i_base_integer.h"
#include "rv64/instructions/rv64m_multiplication.h"

 /**
  * amoswap.w - Atomic Memory Operation SWAP (Word)
  * `amoswap.w rd, rs2, (rs1)`
  * `amoswap.w.aq rd, rs2, (rs1)`
  * `amoswap.w.rl rd, rs2, (rs1)`
  * Atomically swaps the value in memory at address rs1 and the value in register rs2.
  * All `AMO` operations have lock aquire and release capabilities that are currently
  * ignored by this emulator (emulator is currently a single hart).
  */
static inline void rv64a_amoswap_w(
    emulator_rv64_t* emulator,
    uint8_t aquire,
    uint8_t release,
    uint8_t rs2,
    uint8_t rs1,
    uint8_t rd
) {
    uint64_t address = emulator->registers.regs[rs1];
    uint64_t value = emulator->registers.regs[rs2];

    if (aquire) {
        // todo: if multi-threading is added, ...
    }

    int32_t memory_value = 0;
    memcpy(&memory_value, &emulator->memory[address], 4);
    memcpy(&emulator->memory[address], &value, 4);

    if (release) {
        // todo: if multi-threading is added, ...
    }

    if (rd != 0) {
        emulator->registers.regs[rd] = (int64_t)memory_value; // sign extend via cast
    }
}

static inline void rv64_amoadd_w(
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
    emu_result_t result = ER_SUCCESS;

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
            break;
        }
        case I_RV64ZAAMO_AMOADD_W: {
            rv64_amoadd_w(emulator, aquire, release, rs2, rs1, rd);
            break;
        }

        // tags todo:
        case I_RV64ZALRSC_LR_W:
        case I_RV64ZALRSC_SC_W:

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
    return(result);
}
