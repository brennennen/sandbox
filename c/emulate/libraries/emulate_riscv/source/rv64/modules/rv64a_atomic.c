
#include <string.h>

#include "shared/include/result.h"

#include "rv64/rv64_decode.h"
#include "rv64/rv64_hart.h"
#include "rv64/rv64_instructions.h"

/*
 * MARK: alrsc
 */

/*
 * MARK: aamo
 */

/**
 * amoswap.w - Atomic Memory Operation SWAP (Word)
 * `amoswap.w rd, rs2, (rs1)`
 * `amoswap.w.aq rd, rs2, (rs1)`
 * `amoswap.w.rl rd, rs2, (rs1)`
 * Atomically swaps the value in memory at address rs1 and the value in register rs2.
 */
static inline void rv64a_amoswap_w(
    rv64_hart_t* hart,
    uint8_t aquire,
    uint8_t release,
    uint8_t rs2,
    uint8_t rs1,
    uint8_t rd
) {
    uint64_t address = hart->registers[rs1];
    uint64_t value = hart->registers[rs2];

    if (aquire) {
        // todo: if multi-threading is added, ...
    }

    int32_t memory_value = 0;
    memcpy(&memory_value, &hart->shared_system->memory[address], 4);
    memcpy(&hart->shared_system->memory[address], &value, 4);

    if (release) {
        // todo: if multi-threading is added, ...
    }

    if (rd != 0) {
        hart->registers[rd] = (int64_t)memory_value;  // sign extend via cast
    }
}

static inline void rv64_amoadd_w(
    rv64_hart_t* hart,
    uint8_t aquire,
    uint8_t release,
    uint8_t rs2,
    uint8_t rs1,
    uint8_t rd
) {
    // todo
}

/*
 * MARK: awrs
 */

/*
 * MARK: acas
 */

/**
 * amocas.w - Atomic Compare And Swap (Word)
 * `amocas.w rd, rs2, (rs1)`
 * Atomically loads a 32-bit value from address in `rs1`, compares the loaded value to the
 * 32-bit value held in `rd`, and if equal, stores the value held in `rs2` to `rs1`.
 */
static inline void rv64a_amocas_w(
    rv64_hart_t* hart,
    uint8_t aquire,
    uint8_t release,
    uint8_t rs2,
    uint8_t rs1,
    uint8_t rd
) {
    if (aquire) {
        // todo: if multi-threading is added, ...
    }

    uint64_t address = hart->registers[rs1];
    int32_t expected = (int32_t)hart->registers[rd];
    int32_t original_memory_value = 0;
    memcpy(&original_memory_value, &hart->shared_system->memory[address], 4);
    if (original_memory_value == expected) {
        memcpy(&hart->shared_system->memory[address], &hart->registers[rs2], 4);
    }

    if (release) {
        // todo: if multi-threading is added, ...
    }

    if (rd != 0) {
        hart->registers[rd] = (int64_t)original_memory_value;  // sign extend via cast
    }
}

/*
 * MARK: abha
 */

/*
 * MARK: RV64A Main
 */

emu_result_t rv64a_atomic_emulate(
    rv64_hart_t* hart,
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

    printf(
        "rv64a_atomic_emulate: aquire: %d, release: %d, rs2: %d, rs1: %d, rd: %d\n", aquire,
        release, rs2, rs1, rd
    );

    switch (tag) {
        // alrsc
        case I_RV64ZALRSC_LR_W:
        case I_RV64ZALRSC_SC_W:
        case I_RV64ZALRSC_LR_D:
        case I_RV64ZALRSC_SC_D: {
            printf("%s: todo alrsc\n", __func__);
            result = ER_FAILURE;
            break;
        }
        // aamo
        case I_RV64ZAAMO_AMOSWAP_W: {
            rv64a_amoswap_w(hart, aquire, release, rs2, rs1, rd);
            break;
        }
        case I_RV64ZAAMO_AMOADD_W: {
            rv64_amoadd_w(hart, aquire, release, rs2, rs1, rd);
            break;
        }
        case I_RV64ZAAMO_AMOXOR_W:
        case I_RV64ZAAMO_AMOAND_W:
        case I_RV64ZAAMO_AMOOR_W:
        case I_RV64ZAAMO_AMOMIN_W:
        case I_RV64ZAAMO_AMOMAX_W:
        case I_RV64ZAAMO_AMOMINU_W:
        case I_RV64ZAAMO_AMOMAXU_W:
        case I_RV64ZAAMO_AMOSWAP_D:
        case I_RV64ZAAMO_AMOADD_D:
        case I_RV64ZAAMO_AMOXOR_D:
        case I_RV64ZAAMO_AMOAND_D:
        case I_RV64ZAAMO_AMOOR_D:
        case I_RV64ZAAMO_AMOMIN_D:
        case I_RV64ZAAMO_AMOMAX_D:
        case I_RV64ZAAMO_AMOMINU_D:
        case I_RV64ZAAMO_AMOMAXU_D: {
            printf("%s: todo aamo\n", __func__);
            result = ER_FAILURE;
            break;
        }
        // awrs
        case I_RV64ZAWRS_WRS_NTO:
        case I_RV64ZAWRS_WRS_STO: {
            printf("%s: todo awrs\n", __func__);
            result = ER_FAILURE;
            break;
        }
        // acas
        case I_RV64ZACAS_AMOCAS_W: {
            rv64a_amocas_w(hart, aquire, release, rs2, rs1, rd);
            break;
        }
        case I_RV64ZACAS_AMOCAS_D:
        case I_RV64ZACAS_AMOCAS_Q: {
            printf("%s: todo acas\n", __func__);
            result = ER_FAILURE;
            break;
        }
        // abha
        case I_RV64ZABHA_AMOSWAP_B:
        case I_RV64ZABHA_AMOADD_B:
        case I_RV64ZABHA_AMOAND_B:
        case I_RV64ZABHA_AMOOR_B:
        case I_RV64ZABHA_AMOXOR_B:
        case I_RV64ZABHA_AMOMAX_B:
        case I_RV64ZABHA_AMOMAXU_B:
        case I_RV64ZABHA_AMOMIN_B:
        case I_RV64ZABHA_AMOMINU_B:
        case I_RV64ZABHA_AMOCAS_B:
        case I_RV64ZABHA_AMOSWAP_H:
        case I_RV64ZABHA_AMOADD_H:
        case I_RV64ZABHA_AMOAND_H:
        case I_RV64ZABHA_AMOOR_H:
        case I_RV64ZABHA_AMOXOR_H:
        case I_RV64ZABHA_AMOMAX_H:
        case I_RV64ZABHA_AMOMAXU_H:
        case I_RV64ZABHA_AMOMIN_H:
        case I_RV64ZABHA_AMOMINU_H:
        case I_RV64ZABHA_AMOCAS_H: {
            printf("%s: todo abha\n", __func__);
            result = ER_FAILURE;
            break;
        }
            // ...

        default: {
            printf("%s: todo default\n", __func__);
            result = ER_FAILURE;
            break;
        }
    }
    return (result);
}
