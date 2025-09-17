
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "logger.h"

#include "rv64/rv64_common.h"
#include "rv64/rv64_decode.h"
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"

#include "rv64/instructions/rv64m_multiplication.h"

/*
 * MARK: I-Type
 */

/**
 * `flw rd, offset(rs1)`
 * Load a 32-bit word from memory address `rs1 + imm12` into a floating-point register `rd`.
 * @see 20.5. Single-Precision Load and Store Instructions (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_single_precision_load_and_store_instructions)
 */
static inline void rv64f_flw(
    rv64_hart_t* hart,
    uint8_t imm12,
    uint8_t rs1,
    rv64f_width_t width,
    uint8_t rd
) {
    uint64_t base_address = hart->registers[rs1];
    uint64_t effective_address = base_address + imm12;
    switch(width) {
        case RV64F_WIDTH_32: {
            memcpy(&hart->float32_registers[rd],
                &hart->shared_system->memory[effective_address], sizeof(float)
            );
            break;
        }
        default: {
            LOG(LOG_ERROR, "%s:todo: support non-single widths: %d", __func__, width);
            break;
        }
    }
}

emu_result_t rv64f_emulate_i_type(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    int16_t imm12 = 0;
    uint8_t rs1 = 0;
    rv64f_width_t width = 0;
    uint8_t rd = 0;

    rv64f_decode_i_type(raw_instruction, &imm12, &rs1, &width, &rd);

    switch(tag) {
        case I_RV64F_FLW: {
            rv64f_flw(hart, imm12, rs1, width, rd);
            break;
        }
        default: {
            LOG(LOG_ERROR, "%s: instruction not implemented", __func__);
            return(ER_FAILURE);
        }
    }
    return(ER_SUCCESS);
}

/*
 * MARK: S-Type
 */

 /**
 * `fsw rd, offset(rs1)`
 * Stores a 32-bit word from rs1 into a memory address (rs2 + imm12).
 * @see 20.5. Single-Precision Load and Store Instructions (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_single_precision_load_and_store_instructions)
 */
static inline void rv64f_fsw(
    rv64_hart_t* hart,
    int16_t offset,
    uint8_t rs1,
    uint8_t rs2,
    rv64f_width_t width
) {
    uint64_t base_address = hart->registers[rs2];
    uint64_t effective_address = base_address + offset;
    switch(width) {
        case RV64F_WIDTH_32: {
            memcpy(&hart->shared_system->memory[effective_address],
                &hart->float32_registers[rs1], sizeof(float)
            );
            break;
        }
        default: {
            LOG(LOG_ERROR, "%s:todo: support non-single widths: %d", __func__, width);
            break;
        }
    }
}

emu_result_t rv64_emulate_s_type(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    int16_t offset = 0;
    uint8_t rs1 = 0;
    uint8_t rs2 = 0;
    uint8_t width = 0;

    rv64f_decode_s_type(raw_instruction, &offset, &rs1, &rs2, &width);

    switch(tag) {
        case I_RV64F_FSW: {
            rv64f_fsw(hart, offset, rs1, rs2, width);
            break;
        }
        default: {
            LOG(LOG_ERROR, "rv64f_emulate_s_type: instruction not implemented");
            return(ER_FAILURE);
        }
    }
    return(ER_SUCCESS);
}

/*
 * MARK: R4-Type
 */

static inline void rv64f_fmadd_s(rv64_hart_t* hart, int32_t rs1, uint8_t rs2, uint8_t rs3, uint8_t rd) {
    printf("todo: rv64f_fmadd_s\n");
}

static inline void rv64f_fmsub_s(rv64_hart_t* hart, int32_t rs1, uint8_t rs2, uint8_t rs3, uint8_t rd) {
    printf("todo: rv64f_fmsub_s\n");
}

static inline void rv64f_fnmsub_s(rv64_hart_t* hart, int32_t rs1, uint8_t rs2, uint8_t rs3, uint8_t rd) {
    printf("todo: rv64f_fnmsub_s\n");
}

static inline void rv64f_fnmadd_s(rv64_hart_t* hart, int32_t rs1, uint8_t rs2, uint8_t rs3, uint8_t rd) {
    printf("todo: rv64f_fnmadd_s\n");
}

emu_result_t rv64_emulate_r4_type(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    uint8_t rs1 = 0;
    uint8_t rs2 = 0;
    uint8_t rs3 = 0;
    uint8_t rd = 0;

    // todo
    //rv64_decode_r4_type(raw_instruction, &imm12, &rs1, &rs2);

    switch(tag) {
        case I_RV64F_FMADD_S: {
            rv64f_fmadd_s(hart, rs1, rs2, rs3, rd);
            break;
        }
        case I_RV64F_FMSUB_S: {
            rv64f_fmsub_s(hart, rs1, rs2, rs3, rd);
            break;
        }
        case I_RV64F_FNMSUB_S: {
            rv64f_fnmsub_s(hart, rs1, rs2, rs3, rd);
            break;
        }
        case I_RV64F_FNMADD_S: {
            rv64f_fnmadd_s(hart, rs1, rs2, rs3, rd);
            break;
        }
        default: {
            LOG(LOG_ERROR, "rv64_emulate_r4_type: instruction not implemented");
            return(ER_FAILURE);
        }
    }
    return(ER_SUCCESS);
}


/*
 * MARK: R-Type
 */

 /**
  * fadd.s - Float ADD (Single-precision)
  * `fadd.s rd, rs1, rs2`
  * Add 2 single-precision floating point numbers from float registers together and
  * store the result in a float register.
  * @see 20.6. Single-Precision Floating-Point Computational Instructions (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#single-float-compute)
  */
static inline void rv64f_fadd_s(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rm, uint8_t rd) {
    float f1 = hart->float32_registers[rs1];
    float f2 = hart->float32_registers[rs2];
    // todo: rounding mode
    // todo: update fflag exceptions
    hart->float32_registers[rd] = f1 + f2;
}

static inline void rv64f_fsub_s(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rm, uint8_t rd) {
    printf("todo: rv64f_fsub_s\n");
}

static inline void rv64f_fmul_s(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rm, uint8_t rd) {
    printf("todo: rv64f_fmul_s\n");
}

static inline void rv64f_fdiv_s(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rm, uint8_t rd) {
    printf("todo: rv64f_fdiv_s\n");
}

static inline void rv64f_fsqrt_s(rv64_hart_t* hart, uint8_t rs1, uint8_t rm, uint8_t rd) {
    printf("todo: rv64f_fsqrt_s\n");
}

static inline void rv64f_fsgnj_s(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    printf("todo: rv64f_fsgnj_s\n");
}

static inline void rv64f_fsgnjn_s(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    printf("todo: rv64f_fsgnjn_s\n");
}

static inline void rv64f_fsgnjx_s(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    printf("todo: rv64f_fsgnjx_s\n");
}

static inline void rv64f_fmin_s(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    printf("todo: rv64f_fmin_s\n");
}

static inline void rv64f_fmax_s(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    printf("todo: rv64f_fmin_s\n");
}

static inline void rv64f_fcvt_w_s(rv64_hart_t* hart, uint8_t rs1, uint8_t rm, uint8_t rd) {
    printf("todo: rv64f_fcvt_w_s\n");
}

static inline void rv64f_fcvt_wu_s(rv64_hart_t* hart, uint8_t rs1, uint8_t rm, uint8_t rd) {
    printf("todo: rv64f_fcvt_wu_s\n");
}

static inline void rv64f_fmv_x_w(rv64_hart_t* hart, uint8_t rs1, uint8_t rd) {
    printf("todo: rv64f_fmv_x_w\n");
}

static inline void rv64f_feq_s(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    printf("todo: rv64f_feq_s\n");
}

static inline void rv64f_flt_s(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    printf("todo: rv64f_flt_s\n");
}

static inline void rv64f_fle_s(rv64_hart_t* hart, uint8_t rs1, uint8_t rs2, uint8_t rd) {
    printf("todo: rv64f_fle_s\n");
}

static inline void rv64f_fclass_s(rv64_hart_t* hart, uint8_t rs1, uint8_t rd) {
    printf("todo: rv64f_fclass_s\n");
}

static inline void rv64f_fcvt_s_w(rv64_hart_t* hart, uint8_t rs1, uint8_t rm, uint8_t rd) {
    printf("todo: rv64f_fcvt_s_w\n");
}

static inline void rv64f_fcvt_s_wu(rv64_hart_t* hart, uint8_t rs1, uint8_t rm, uint8_t rd) {
    printf("todo: rv64f_fcvt_s_wu\n");
}

static inline void rv64f_fmv_w_x(rv64_hart_t* hart, uint8_t rs1, uint8_t rd) {
    printf("todo: rv64f_fmv_w_x\n");
}

static inline void rv64f_fcvt_l_s(rv64_hart_t* hart, uint8_t rs1, uint8_t rm, uint8_t rd) {
    printf("todo: rv64f_fcvt_l_s\n");
}

static inline void rv64f_fcvt_lu_s(rv64_hart_t* hart, uint8_t rs1, uint8_t rm, uint8_t rd) {
    printf("todo: rv64f_fcvt_lu_s\n");
}

static inline void rv64f_fcvt_s_l(rv64_hart_t* hart, uint8_t rs1, uint8_t rm, uint8_t rd) {
    printf("todo: rv64f_fcvt_s_l\n");
}

static inline void rv64f_fcvt_s_lu(rv64_hart_t* hart, uint8_t rs1, uint8_t rm, uint8_t rd) {
    printf("todo: rv64f_fcvt_s_lu\n");
}

emu_result_t rv64f_emulate_r_type(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    uint8_t rs1 = 0;
    uint8_t rs2 = 0;
    uint8_t rm = 0;
    uint8_t rd = 0;

    rv64f_decode_r_type(raw_instruction, &rs2, &rs1, &rm, &rd);

    switch(tag) {
        case I_RV64F_FADD_S: {
            rv64f_fadd_s(hart, rs1, rs2, rm, rd);
            break;
        }
        case I_RV64F_FSUB_S: {
            rv64f_fsub_s(hart, rs1, rs2, rm, rd);
            break;
        }
        case I_RV64F_FMUL_S: {
            rv64f_fmul_s(hart, rs1, rs2, rm, rd);
            break;
        }
        case I_RV64F_FDIV_S: {
            rv64f_fdiv_s(hart, rs1, rs2, rm, rd);
            break;
        }
        case I_RV64F_FSQRT_S: {
            rv64f_fsqrt_s(hart, rs1, rm, rd);
            break;
        }
        case I_RV64F_FSGNJ_S: {
            rv64f_fsgnj_s(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64F_FSGNJN_S: {
            rv64f_fsgnjn_s(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64F_FSGNJX_S: {
            rv64f_fsgnjx_s(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64F_FMIN_S: {
            rv64f_fmin_s(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64F_FMAX_S: {
            rv64f_fmax_s(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64F_FCVT_W_S: {
            rv64f_fcvt_w_s(hart, rs1, rm, rd);
            break;
        }
        case I_RV64F_FCVT_WU_S: {
            rv64f_fcvt_wu_s(hart, rs1, rm, rd);
            break;
        }
        case I_RV64F_FMV_X_W: {
            rv64f_fmv_x_w(hart, rs1, rd);
            break;
        }
        case I_RV64F_FEQ_S: {
            rv64f_feq_s(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64F_FLT_S: {
            rv64f_flt_s(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64F_FLE_S: {
            rv64f_fle_s(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64F_FCLASS_S: {
            rv64f_fclass_s(hart, rs1, rd);
            break;
        }
        case I_RV64F_FCVT_S_W: {
            rv64f_fcvt_s_w(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64F_FCVT_S_WU: {
            rv64f_fcvt_s_wu(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64F_FMV_W_X: {
            rv64f_fmv_w_x(hart, rs1, rd);
            break;
        }
        case I_RV64F_FCVT_L_S: {
            rv64f_fcvt_l_s(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64F_FCVT_LU_S: {
            rv64f_fcvt_lu_s(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64F_FCVT_S_L: {
            rv64f_fcvt_s_l(hart, rs1, rs2, rd);
            break;
        }
        case I_RV64F_FCVT_S_LU: {
            rv64f_fcvt_s_lu(hart, rs1, rs2, rd);
            break;
        }
        default: {
            LOG(LOG_ERROR, "rv64_emulate_r4_type: instruction not implemented");
            return(ER_FAILURE);
        }
    }
    return(ER_SUCCESS);
}



/*
 * MARK: rv64f_float_emulate
 */
emu_result_t rv64f_float_emulate(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    switch(tag) {
        // Core Format "I" - "register-immediate"
        case I_RV64F_FLW: {
            rv64f_emulate_i_type(hart, raw_instruction, tag);
            break;
        }
        // Core Format "S" - "store"
        case I_RV64F_FSW: {
            rv64_emulate_s_type(hart, raw_instruction, tag);
            break;
        }
        // Core Format "R4"
        case I_RV64F_FMADD_S:
        case I_RV64F_FMSUB_S:
        case I_RV64F_FNMSUB_S:
        case I_RV64F_FNMADD_S: {
            rv64_emulate_r4_type(hart, raw_instruction, tag);
            break;
        }
        // Core Format "R"
        case I_RV64F_FADD_S:
        case I_RV64F_FSUB_S:
        case I_RV64F_FMUL_S:
        case I_RV64F_FDIV_S:
        case I_RV64F_FSQRT_S:
        case I_RV64F_FSGNJ_S:
        case I_RV64F_FSGNJN_S:
        case I_RV64F_FSGNJX_S:
        case I_RV64F_FMIN_S:
        case I_RV64F_FMAX_S:
        case I_RV64F_FCVT_W_S:
        case I_RV64F_FCVT_WU_S:
        case I_RV64F_FMV_X_W:
        case I_RV64F_FEQ_S:
        case I_RV64F_FLT_S:
        case I_RV64F_FLE_S:
        case I_RV64F_FCLASS_S:
        case I_RV64F_FCVT_S_W:
        case I_RV64F_FCVT_S_WU:
        case I_RV64F_FMV_W_X:
        case I_RV64F_FCVT_L_S:
        case I_RV64F_FCVT_LU_S:
        case I_RV64F_FCVT_S_L:
        case I_RV64F_FCVT_S_LU: {
            rv64f_emulate_r_type(hart, raw_instruction, tag);
            break;
        }
        default: {
            LOG(LOG_ERROR, "rv64f_float_emulate: instruction not implemented");
            return(ER_FAILURE);
        }
    }
    return(ER_SUCCESS);
}


