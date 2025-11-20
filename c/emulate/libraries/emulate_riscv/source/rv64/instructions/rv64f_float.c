#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <fenv.h>
#include <math.h>

#include "logger.h"

#include "rv64/rv64_common.h"
#include "rv64/rv64_decode.h"
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"

#include "rv64/instructions/rv64f_float.h"

static void rv64f_update_fcsr_flags(rv64_hart_t* hart, int fenv_flags) {
    uint64_t riscv_flags = 0;

    if (fenv_flags & FE_INEXACT) {
        riscv_flags |= (1 << RV64F_FCSR_INEXACT);
    }
    if (fenv_flags & FE_UNDERFLOW) {
        riscv_flags |= (1 << RV64F_FCSR_UNDERFLOW);
    }
    if (fenv_flags & FE_OVERFLOW) {
        riscv_flags |= (1 << RV64F_FCSR_OVERFLOW);
    }
    if (fenv_flags & FE_DIVBYZERO) {
        riscv_flags |= (1 << RV64F_FCSR_DIVIDE_BY_ZERO);
    }
    if (fenv_flags & FE_INVALID) {
        riscv_flags |= (1 << RV64F_FCSR_INVALID_OPERATION);
    }
    
    // TODO: set fflags too? or just use fcsr?
    hart->csrs.fcsr |= riscv_flags;
}

/*
 * MARK: I-Type
 */

/**
 * `flw rd, offset(rs1)`
 * Load a 32-bit word from memory address `rs1 + imm12` into a floating-point register `rd`.
 * @see 20.5. Single-Precision Load and Store Instructions (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_single_precision_load_and_store_instructions)
 */
static void rv64f_flw(
    rv64_hart_t* hart,
    uint8_t rd,
    uint8_t imm12,
    uint8_t rs1
) {
    uint64_t base_address = hart->registers[rs1];
    uint64_t effective_address = base_address + imm12;
    memcpy(&hart->float32_registers[rd],
        &hart->shared_system->memory[effective_address],
        sizeof(float)
    );
}

emu_result_t rv64f_emulate_i_type(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    int16_t imm12 = 0;
    uint8_t rs1 = 0;
    uint8_t width = 0;
    uint8_t rd = 0;

    rv64f_decode_i_type(raw_instruction, &imm12, &rs1, &width, &rd);

    switch(tag) {
        case I_RV64F_FLW: {
            rv64f_flw(hart, rd, imm12, rs1);
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
 * `fsw rs2, offset(rs1)`
 * Stores a 32-bit word from rs1 into a memory address (rs2 + imm12).
 * @see 20.5. Single-Precision Load and Store Instructions (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_single_precision_load_and_store_instructions)
 */
static void rv64f_fsw(
    rv64_hart_t* hart,
    uint8_t rs2,
    int16_t offset,
    uint8_t rs1
) {
    uint64_t base_address = hart->registers[rs2];
    uint64_t effective_address = base_address + offset;
    memcpy(&hart->shared_system->memory[effective_address],
        &hart->float32_registers[rs1], sizeof(float)
    );
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
            rv64f_fsw(hart, rs2, offset, rs1);
            break;
        }
        default: {
            LOG(LOG_ERROR, "rv64f_emulate_s_type: instruction not implemented");
            return(ER_FAILURE);
        }
    }
    return(ER_SUCCESS);
}

/**
 * Sets c library floating point environment variables to describe how to
 * handle rounding based on an instructions `rm` (rounding mode) or the
 * rounding mode CSR if a dynamic rounding mode is requested.
 * 
 * @return Target rounding mode, either the rm passed in or the csr if
 *          rm is set to dynamic. Can be ignored.
 */
static rv64f_rounding_mode_t rv64f_set_fenv_rounding_mode(
    rv64_hart_t* hart,
    rv64f_rounding_mode_t rounding_mode
) {
    rv64f_rounding_mode_t target_rounding_mode = rounding_mode;
    if (target_rounding_mode == RV64F_ROUND_DYNAMIC) {
        target_rounding_mode = (hart->csrs.fcsr >> 5) & 0x7; // TODO: break csrs out into structs?
    }

    switch(target_rounding_mode) {
        case RV64F_ROUND_TO_NEAREST_TIES_EVEN: {
            fesetround(FE_TONEAREST);
            break;
        }
        case RV64F_ROUND_TOWARDS_ZERO: {
            fesetround(FE_TOWARDZERO);
            break;
        }
        case RV64F_ROUND_DOWN: {
            fesetround(FE_DOWNWARD);
            break;
        }
        case RV64F_ROUND_UP: {
            fesetround(FE_UPWARD);
            break;
        }
        case RV64F_ROUND_TO_NEAREST_TIES_MAX_MAGNITUDE: {
            // TODO! no fenv.h equivalent, fall through for now
        }
        // TODO
        default: {
            printf("%s: rounding mode not supported! rm: %d (%d)\n",
                __func__, rounding_mode, target_rounding_mode);
            fesetround(FE_TONEAREST);
            break;
        }
    }
    return(target_rounding_mode);
}

/*
 * MARK: R4-Type
 */

/**
 * `fmadd.s rd, rs1, rs2, rs3`
 * `fmadd.s rd, rs1, rs2, rs3, rm`
 * Single-precision fused multiply addition `rd = (rs1 * rs2) + rs3`.
 * Multiplies rs1 and rs2, then adds rs3, then rounds. "Fused" = rounding occurs once at the
 * end.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#single-float-compute
 */
static void rv64f_fmadd_s(
    rv64_hart_t* hart,
    uint8_t rd,
    uint8_t rs1,
    uint8_t rs2,
    uint8_t rs3,
    rv64f_rounding_mode_t rm
) {
    int host_rounding_mode = fegetround();
    (void)rv64f_set_fenv_rounding_mode(hart, rm);
    feclearexcept(FE_ALL_EXCEPT);
    hart->float32_registers[rd] = fmaf(
        hart->float32_registers[rs1],
        hart->float32_registers[rs2],
        hart->float32_registers[rs3]
    );
    rv64f_update_fcsr_flags(hart, fetestexcept(FE_ALL_EXCEPT));
    fesetround(host_rounding_mode); // reset back to the original host rounding mode.
}

/**
 * `fmsub.s rd, rs1, rs2, rs3`
 * `fmsub.s rd, rs1, rs2, rs3, rm`
 * Single-precision fused multiply subtract `rd = (rs1 * rs2) - rs3`.
 * Multiplies rs1 and rs2, then subtracts rs3, then rounds. "Fused" = rounding occurs
 * once at the end.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#single-float-compute
 */
static void rv64f_fmsub_s(
    rv64_hart_t* hart,
    uint8_t rd,
    uint8_t rs1,
    uint8_t rs2,
    uint8_t rs3,
    uint8_t rm
) {
    int host_rounding_mode = fegetround();
    (void)rv64f_set_fenv_rounding_mode(hart, rm);
    feclearexcept(FE_ALL_EXCEPT);
    hart->float32_registers[rd] = fmaf(
        hart->float32_registers[rs1],
        hart->float32_registers[rs2],
        (hart->float32_registers[rs3] * -1)
    );
    rv64f_update_fcsr_flags(hart, fetestexcept(FE_ALL_EXCEPT));
    fesetround(host_rounding_mode); // reset back to the original host rounding mode.
}

/**
 * `fnmsub.s rd, rs1, rs2, rs3`
 * `fnmsub.s rd, rs1, rs2, rs3, rm`
 * Single-precision fused negated multiply subtract `rd = -(rs1 * rs2)) + rs3`.
 * WARNING: FNMSUB and FNMADD ARE COUNTERINTUITIVELY NAMED, NOTE THE "+ rs3".
 * Multiplies rs1 and rs2, then multiplies by -1, then adds rs3, then rounds.
 * "Fused" = rounding occurs once at the end.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#single-float-compute
 */
static void rv64f_fnmsub_s(
    rv64_hart_t* hart,
    uint8_t rd,
    uint8_t rs1,
    uint8_t rs2,
    uint8_t rs3,
    uint8_t rm
) {
    int host_rounding_mode = fegetround();
    (void)rv64f_set_fenv_rounding_mode(hart, rm);
    feclearexcept(FE_ALL_EXCEPT);
    // TODO: currently not following spec order of operations, investigate if this is ok.
    // rd = ((-1 * rs1) * rs2)) + rs3
    hart->float32_registers[rd] = fmaf(
        (hart->float32_registers[rs1] * -1),
        hart->float32_registers[rs2],
        hart->float32_registers[rs3]
    );
    rv64f_update_fcsr_flags(hart, fetestexcept(FE_ALL_EXCEPT));
    fesetround(host_rounding_mode); // reset back to the original host rounding mode.
}

/**
 * `fnmadd.s rd, rs1, rs2, rs3`
 * `fnmadd.s rd, rs1, rs2, rs3, rm`
 * Single-precision fused negated multiply subtract `rd = -(rs1 * rs2)) - rs3`.
 * WARNING: FNMSUB and FNMADD ARE COUNTERINTUITIVELY NAMED, NOTE THE "- rs3".
 * Multiplies rs1 and rs2, then multiplies by -1, then adds rs3, then rounds.
 * "Fused" = rounding occurs once at the end.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#single-float-compute
 */
static void rv64f_fnmadd_s(
    rv64_hart_t* hart,
    uint8_t rd,
    uint8_t rs1,
    uint8_t rs2,
    uint8_t rs3,
    uint8_t rm
) {
    int host_rounding_mode = fegetround();
    (void)rv64f_set_fenv_rounding_mode(hart, rm);
    feclearexcept(FE_ALL_EXCEPT);
    // TODO: currently not following spec order of operations, investigate if this is ok.
    // rd = ((-1 * rs1) * rs2)) - rs3
    hart->float32_registers[rd] = fmaf(
        (hart->float32_registers[rs1]* -1),
        hart->float32_registers[rs2],
        (hart->float32_registers[rs3] * -1)
    );
    rv64f_update_fcsr_flags(hart, fetestexcept(FE_ALL_EXCEPT));
    fesetround(host_rounding_mode); // reset back to the original host rounding mode.
}

emu_result_t rv64_emulate_r4_type(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    uint8_t rs3 = 0;
    uint8_t fmt = 0;
    uint8_t rs2 = 0;
    uint8_t rs1 = 0;
    uint8_t rm = 0;
    uint8_t rd = 0;

    rv64_decode_r4_type(raw_instruction, &rs3, &fmt, &rs2, &rs1, &rm, &rd);

    switch(tag) {
        case I_RV64F_FMADD_S: {
            rv64f_fmadd_s(hart, rd, rs1, rs2, rs3, rm);
            break;
        }
        case I_RV64F_FMSUB_S: {
            rv64f_fmsub_s(hart, rd, rs1, rs2, rs3, rm);
            break;
        }
        case I_RV64F_FNMSUB_S: {
            rv64f_fnmsub_s(hart, rd, rs1, rs2, rs3, rm);
            break;
        }
        case I_RV64F_FNMADD_S: {
            rv64f_fnmadd_s(hart, rd, rs1, rs2, rs3, rm);
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
static void rv64f_fadd_s(rv64_hart_t* hart, uint8_t rd, uint8_t rs1, uint8_t rs2, uint8_t rm) {
    int host_rounding_mode = fegetround();
    (void)rv64f_set_fenv_rounding_mode(hart, rm);
    feclearexcept(FE_ALL_EXCEPT);
    hart->float32_registers[rd] = hart->float32_registers[rs1] 
                                + hart->float32_registers[rs2];
    rv64f_update_fcsr_flags(hart, fetestexcept(FE_ALL_EXCEPT));
    fesetround(host_rounding_mode); // reset back to the original host rounding mode.
}

static void rv64f_fsub_s(rv64_hart_t* hart, uint8_t rd, uint8_t rs1, uint8_t rs2, uint8_t rm) {
    int host_rounding_mode = fegetround();
    (void)rv64f_set_fenv_rounding_mode(hart, rm);
    feclearexcept(FE_ALL_EXCEPT);
    hart->float32_registers[rd] = hart->float32_registers[rs1] 
                                - hart->float32_registers[rs2];
    rv64f_update_fcsr_flags(hart, fetestexcept(FE_ALL_EXCEPT));
    fesetround(host_rounding_mode); // reset back to the original host rounding mode.
}

static void rv64f_fmul_s(rv64_hart_t* hart, uint8_t rd, uint8_t rs1, uint8_t rs2, uint8_t rm) {
    int host_rounding_mode = fegetround();
    (void)rv64f_set_fenv_rounding_mode(hart, rm);
    feclearexcept(FE_ALL_EXCEPT);
    hart->float32_registers[rd] = hart->float32_registers[rs1] 
                                * hart->float32_registers[rs2];
    rv64f_update_fcsr_flags(hart, fetestexcept(FE_ALL_EXCEPT));
    fesetround(host_rounding_mode); // reset back to the original host rounding mode.
}

static void rv64f_fdiv_s(rv64_hart_t* hart, uint8_t rd, uint8_t rs1, uint8_t rs2, uint8_t rm) {
    int host_rounding_mode = fegetround();
    (void)rv64f_set_fenv_rounding_mode(hart, rm);
    feclearexcept(FE_ALL_EXCEPT);
    hart->float32_registers[rd] = hart->float32_registers[rs1] 
                                / hart->float32_registers[rs2];
    rv64f_update_fcsr_flags(hart, fetestexcept(FE_ALL_EXCEPT));
    fesetround(host_rounding_mode); // reset back to the original host rounding mode.
}

static void rv64f_fsqrt_s(rv64_hart_t* hart, uint8_t rd, uint8_t rs1, uint8_t rm) {
    int host_rounding_mode = fegetround();
    (void)rv64f_set_fenv_rounding_mode(hart, rm);
    feclearexcept(FE_ALL_EXCEPT);
    hart->float32_registers[rd] = sqrt(hart->float32_registers[rs1]);
    rv64f_update_fcsr_flags(hart, fetestexcept(FE_ALL_EXCEPT));
    fesetround(host_rounding_mode); // reset back to the original host rounding mode.
}

/**
 * fsgnj.s - Float SiGN inJection (Single-precision)
 * `fsgnj.s rd, rs1, rs2`
 * Takes the sign bit of rs2 and applies it to rs1, putting the result in rd.
 * @see 20.7. Single-Precision Floating-Point Conversion and Move Instructions (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_single_precision_floating_point_conversion_and_move_instructions)
 */
static void rv64f_fsgnj_s(rv64_hart_t* hart, uint8_t rd, uint8_t rs1, uint8_t rs2) {
    hart->float32_registers[rd] = copysignf(
        fabsf(hart->float32_registers[rs1]),
        hart->float32_registers[rs2]
    );
}

/**
 * fsgnjn.s - Float SiGN inJection Negated (Single-precision)
 * `fsgnjn.s rd, rs1, rs2`
 * Takes the opposite sign bit of rs2 and applies it to rs1, putting the result in rd.
 * @see 20.7. Single-Precision Floating-Point Conversion and Move Instructions (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_single_precision_floating_point_conversion_and_move_instructions)
 */
static void rv64f_fsgnjn_s(rv64_hart_t* hart, uint8_t rd, uint8_t rs1, uint8_t rs2) {
    hart->float32_registers[rd] = copysignf(
        fabsf(hart->float32_registers[rs1]),
        (-1 * hart->float32_registers[rs2])
    );
}

/**
 * fsgnjx.s - Float SiGN inJection Xor (Single-precision)
 * `fsgnjx.s rd, rs1, rs2`
 * Takes the sign bit of `rs1 xor rs2` and applies it to rs1, putting the result in rd.
 * @see 20.7. Single-Precision Floating-Point Conversion and Move Instructions (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_single_precision_floating_point_conversion_and_move_instructions)
 */
static void rv64f_fsgnjx_s(rv64_hart_t* hart, uint8_t rd, uint8_t rs1, uint8_t rs2) {
    hart->float32_registers[rd] = copysignf(
        fabsf(hart->float32_registers[rs1]),
        (hart->float32_registers[rs1] * hart->float32_registers[rs2])
    );
}

static void rv64f_fmin_s(rv64_hart_t* hart, uint8_t rd, uint8_t rs1, uint8_t rs2) {
    feclearexcept(FE_ALL_EXCEPT);
    // TODO: handle NaNs and 0.0 vs -0.0 per spec
    hart->float32_registers[rd] = fminf(hart->float32_registers[rs1], hart->float32_registers[rs2]);
    rv64f_update_fcsr_flags(hart, fetestexcept(FE_ALL_EXCEPT));
}

static void rv64f_fmax_s(rv64_hart_t* hart, uint8_t rd, uint8_t rs1, uint8_t rs2) {
    feclearexcept(FE_ALL_EXCEPT);
    // TODO: handle NaNs and 0.0 vs -0.0 per spec
    hart->float32_registers[rd] = fmaxf(hart->float32_registers[rs1], hart->float32_registers[rs2]);
    rv64f_update_fcsr_flags(hart, fetestexcept(FE_ALL_EXCEPT));
}

static float rv64f_round_to_nearest_ties_max_magnitude(float val) {
    if (val > 0.0f) {
        return(floorf(val + 0.5f));
    } else {
        return(ceilf(val - 0.5f));
    }
}

static float rv64f_nearbyintf(float val, rv64f_rounding_mode_t rm) {
    if (rm == RV64F_ROUND_TO_NEAREST_TIES_MAX_MAGNITUDE) {
        return rv64f_round_to_nearest_ties_max_magnitude(val);
    } else {
        // let fenv handle the other cases (fesetround called in `rv64f_set_fenv_rounding_mode`).
        return(nearbyintf(val));
    }
}

/**
 * fcvt.w.s - Floating-point ConVert to signed Word from Single-precision
 * `fcvt.w.s rd, rs1, [rm]`
 * Convert a single-precision floating-point number from a fpu register into 
 * a 32-bit signed integer general purpose register.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_single_precision_floating_point_conversion_and_move_instructions
 */
static void rv64f_fcvt_w_s(rv64_hart_t* hart, uint8_t rd, uint8_t rs1, uint8_t rm) {
    float rs1_float = (float) hart->float32_registers[rs1];
    float rs1_float_rounded = 0.0f;
    int32_t result_int = 0;
    if (isnan(rs1_float) || isinf(rs1_float)) {
        hart->csrs.fcsr |= RV64F_FCSR_INVALID_OPERATION;
        result_int = INT32_MAX;
    } else {
        int host_rounding_mode = fegetround();
        rv64f_rounding_mode_t target_rm = rv64f_set_fenv_rounding_mode(hart, rm);
        rs1_float_rounded = rv64f_nearbyintf(rs1_float, target_rm);

        if (rs1_float_rounded != rs1_float) {
            hart->csrs.fcsr |= RV64F_FCSR_INEXACT;
        }

        if (rs1_float_rounded > (float)INT32_MAX) {
            hart->csrs.fcsr |= RV64F_FCSR_INVALID_OPERATION;
            result_int = INT32_MAX;
        } else if (rs1_float_rounded < (float)INT32_MIN) {
            hart->csrs.fcsr |= RV64F_FCSR_INVALID_OPERATION;
            result_int = INT32_MIN;
        } else {
            result_int = (int32_t) rs1_float_rounded;
        }
        
        fesetround(host_rounding_mode); // reset back to the original host rounding mode.
    }
    hart->registers[rd] = (uint64_t)(int64_t)result_int; // double cast for sign extension
}

static void rv64f_fcvt_wu_s(rv64_hart_t* hart, uint8_t rd, uint8_t rs1, uint8_t rm) {
    printf("todo: rv64f_fcvt_wu_s\n");
}

static void rv64f_fmv_x_w(rv64_hart_t* hart, uint8_t rd, uint8_t rs1) {
    printf("todo: rv64f_fmv_x_w\n");
}

/**
 * feq.s - Float EQuality
 * `feq.s rd, rs1, rs2`
 * Compares 2 floats, sets rd to 1 if these floats are equal, sets rd to 0 otherwise.
 * @see 20.8. Single-Precision Floating-Point Compare Instructions (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_single_precision_floating_point_compare_instructions)
 */
static void rv64f_feq_s(rv64_hart_t* hart, uint8_t rd, uint8_t rs1, uint8_t rs2) {
    feclearexcept(FE_ALL_EXCEPT);
    hart->registers[rd] = (hart->float32_registers[rs1] == hart->float32_registers[rs2]);
    // TODO: handle qNan and sNaN
    rv64f_update_fcsr_flags(hart, fetestexcept(FE_ALL_EXCEPT));
}

/**
 * flt.s - Float Less Than
 * `flt.s rd, rs1, rs2`
 * Compares 2 floats, sets rd to 1 if the first float is less than the second float, 
 * sets rd to 0 otherwise.
 * @see 20.8. Single-Precision Floating-Point Compare Instructions (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_single_precision_floating_point_compare_instructions)
 */
static void rv64f_flt_s(rv64_hart_t* hart, uint8_t rd, uint8_t rs1, uint8_t rs2) {
    feclearexcept(FE_ALL_EXCEPT);
    hart->registers[rd] = (hart->float32_registers[rs1] < hart->float32_registers[rs2]);
    // TODO: handle qNan and sNaN
    rv64f_update_fcsr_flags(hart, fetestexcept(FE_ALL_EXCEPT));
}

/**
 * fle.s - Float Less than or Equal
 * `fle.s rd, rs1, rs2`
 * Compares 2 floats, sets rd to 1 if the first float is less than or equal to the second 
 * float, sets rd to 0 otherwise.
 * @see 20.8. Single-Precision Floating-Point Compare Instructions (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_single_precision_floating_point_compare_instructions)
 */
static void rv64f_fle_s(rv64_hart_t* hart, uint8_t rd, uint8_t rs1, uint8_t rs2) {
    feclearexcept(FE_ALL_EXCEPT);
    hart->registers[rd] = (hart->float32_registers[rs1] <= hart->float32_registers[rs2]);
    // TODO: handle qNan and sNaN
    rv64f_update_fcsr_flags(hart, fetestexcept(FE_ALL_EXCEPT));
}




/**
 * fclass.s - Float CLASSify
 * `fclass.s rd, rs1`
 * Classifies a float into one of a 9 categories and stores that classification in rd.
 * @see 20.9 Single-Precision Floating-Point Classify Instruction (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_single_precision_floating_point_classify_instruction)
 */
static void rv64f_fclass_s(rv64_hart_t* hart, uint8_t rd, uint8_t rs1) {
    int fpclass = fpclassify(hart->float32_registers[rs1]);

    if (isnan(hart->float32_registers[rs1])) {
        hart->registers[rd] = RV64F_CLASS_SIGNALING_NAN;
    }
    // TODO: quiet nan?

    if (hart->float32_registers[rs1] < 0.0f) {
        if (fpclass == FP_INFINITE) {
            hart->registers[rd] = RV64F_CLASS_NEGATIVE_INFINITY;
        } else if (fpclass == FP_NORMAL){
            hart->registers[rd] = RV64F_CLASS_NEGATIVE_NORMAL;
        } else if (fpclass == FP_SUBNORMAL) {
            hart->registers[rd] = RV64F_CLASS_NEGATIVE_SUBNORMAL;
        } else if (fpclass == FP_ZERO) {
            hart->registers[rd] = RV64F_CLASS_NEGATIVE_ZERO;
        }
    } else {
        if (fpclass == FP_INFINITE) {
            hart->registers[rd] = RV64F_CLASS_POSITIVE_INFINITY;
        } else if (fpclass == FP_NORMAL){
            hart->registers[rd] = RV64F_CLASS_POSITIVE_NORMAL;
        } else if (fpclass == FP_SUBNORMAL) {
            hart->registers[rd] = RV64F_CLASS_POSITIVE_SUBNORMAL;
        } else if (fpclass == FP_ZERO) {
            hart->registers[rd] = RV64F_CLASS_POSITIVE_ZERO;
        }
    }
}

static void rv64f_fcvt_s_w(rv64_hart_t* hart, uint8_t rd, uint8_t rs1, uint8_t rm) {
    printf("todo: rv64f_fcvt_s_w\n");
}

static void rv64f_fcvt_s_wu(rv64_hart_t* hart, uint8_t rd, uint8_t rs1, uint8_t rm) {
    printf("todo: rv64f_fcvt_s_wu\n");
}

static void rv64f_fmv_w_x(rv64_hart_t* hart, uint8_t rd, uint8_t rs1) {
    printf("todo: rv64f_fmv_w_x\n");
}

static void rv64f_fcvt_l_s(rv64_hart_t* hart, uint8_t rd, uint8_t rs1, uint8_t rm) {
    printf("todo: rv64f_fcvt_l_s\n");
}

static void rv64f_fcvt_lu_s(rv64_hart_t* hart, uint8_t rd, uint8_t rs1, uint8_t rm) {
    printf("todo: rv64f_fcvt_lu_s\n");
}

static void rv64f_fcvt_s_l(rv64_hart_t* hart, uint8_t rd, uint8_t rs1, uint8_t rm) {
    printf("todo: rv64f_fcvt_s_l\n");
}

static void rv64f_fcvt_s_lu(rv64_hart_t* hart, uint8_t rd, uint8_t rs1, uint8_t rm) {
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
            rv64f_fadd_s(hart, rd, rs1, rs2, rm);
            break;
        }
        case I_RV64F_FSUB_S: {
            rv64f_fsub_s(hart, rd, rs1, rs2, rm);
            break;
        }
        case I_RV64F_FMUL_S: {
            rv64f_fmul_s(hart, rd, rs1, rs2, rm);
            break;
        }
        case I_RV64F_FDIV_S: {
            rv64f_fdiv_s(hart, rd, rs1, rs2, rm);
            break;
        }
        case I_RV64F_FSQRT_S: {
            rv64f_fsqrt_s(hart, rd, rs1, rm);
            break;
        }
        case I_RV64F_FSGNJ_S: {
            rv64f_fsgnj_s(hart, rd, rs1, rs2);
            break;
        }
        case I_RV64F_FSGNJN_S: {
            rv64f_fsgnjn_s(hart, rd, rs1, rs2);
            break;
        }
        case I_RV64F_FSGNJX_S: {
            rv64f_fsgnjx_s(hart, rd, rs1, rs2);
            break;
        }
        case I_RV64F_FMIN_S: {
            rv64f_fmin_s(hart, rd, rs1, rs2);
            break;
        }
        case I_RV64F_FMAX_S: {
            rv64f_fmax_s(hart, rd, rs1, rs2);
            break;
        }
        case I_RV64F_FCVT_W_S: {
            rv64f_fcvt_w_s(hart, rd, rs1, rm);
            break;
        }
        case I_RV64F_FCVT_WU_S: {
            rv64f_fcvt_wu_s(hart, rd, rs1, rm);
            break;
        }
        case I_RV64F_FMV_X_W: {
            rv64f_fmv_x_w(hart, rd, rs1);
            break;
        }
        case I_RV64F_FEQ_S: {
            rv64f_feq_s(hart, rd, rs1, rs2);
            break;
        }
        case I_RV64F_FLT_S: {
            rv64f_flt_s(hart, rd, rs1, rs2);
            break;
        }
        case I_RV64F_FLE_S: {
            rv64f_fle_s(hart, rd, rs1, rs2);
            break;
        }
        case I_RV64F_FCLASS_S: {
            rv64f_fclass_s(hart, rd, rs1);
            break;
        }
        case I_RV64F_FCVT_S_W: {
            rv64f_fcvt_s_w(hart, rd, rs1, rs2);
            break;
        }
        case I_RV64F_FCVT_S_WU: {
            rv64f_fcvt_s_wu(hart, rd, rs1, rs2);
            break;
        }
        case I_RV64F_FMV_W_X: {
            rv64f_fmv_w_x(hart, rd, rs1);
            break;
        }
        case I_RV64F_FCVT_L_S: {
            rv64f_fcvt_l_s(hart, rd, rs1, rs2);
            break;
        }
        case I_RV64F_FCVT_LU_S: {
            rv64f_fcvt_lu_s(hart, rd, rs1, rs2);
            break;
        }
        case I_RV64F_FCVT_S_L: {
            rv64f_fcvt_s_l(hart, rd, rs1, rs2);
            break;
        }
        case I_RV64F_FCVT_S_LU: {
            rv64f_fcvt_s_lu(hart, rd, rs1, rs2);
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


