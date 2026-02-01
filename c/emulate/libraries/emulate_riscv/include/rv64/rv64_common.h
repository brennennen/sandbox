#ifndef RV64_COMMON_H
#define RV64_COMMON_H

/**
 * Used by most instructions that load/store to memory. 3 bits long.
 * ex: lb, sb, flw, fsw
 * TODO: find ref in spec
 *
 */
typedef enum {
    RV64I_MEMORY_WIDTH_16 = 1,  // half
    RV64I_MEMORY_WIDTH_32 = 2,  // word
    RV64I_MEMORY_WIDTH_64 = 3,  // double
    RV64I_MEMORY_WIDTH_128 = 4  // quad
} rv64i_memory_width_t;

/**
 * Each math operation in the F/H/D/Q modules has a 2 bit width stored in the "fmt"
 * field of the instruction.
 *
 * See Table 27. Format field encoding (0 = S, 1 = D, 2 = H, 3 = Q)
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#single-float-compute
 */
typedef enum {
    RV64F_PRECISION_WIDTH_32 = 0,  // single
    RV64F_PRECISION_WIDTH_64 = 1,  // word
    RV64F_PRECISION_WIDTH_16 = 3,  // double
    RV64F_PRECISION_WIDTH_128 = 4  // quad
} rv64f_precision_width_t;

/**
 *
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_rounding_modes
 */
typedef enum {
    RV64F_ROUND_TO_NEAREST_TIES_EVEN = 0,
    RV64F_ROUND_TOWARDS_ZERO = 1,
    RV64F_ROUND_DOWN = 2,
    RV64F_ROUND_UP = 3,
    RV64F_ROUND_TO_NEAREST_TIES_MAX_MAGNITUDE = 4,
    RV64F_ROUND_DYNAMIC = 7
} rv64f_rounding_mode_t;

/**
 * `fcsr` bit flags.
 * @see
 * https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_floating_point_control_and_status_register
 */
typedef enum {
    RV64F_FCSR_INEXACT = 0,           // NX
    RV64F_FCSR_UNDERFLOW = 1,         // UF
    RV64F_FCSR_OVERFLOW = 2,          // OF
    RV64F_FCSR_DIVIDE_BY_ZERO = 3,    // DZ
    RV64F_FCSR_INVALID_OPERATION = 4  // NV
} rv64f_fcsr_flags_t;

#endif  // RV64_COMMON_H
