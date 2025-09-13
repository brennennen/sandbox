#ifndef RV64_VIRTUAL_HARDWARE_CONF_H
#define RV64_VIRTUAL_HARDWARE_CONF_H

/**
 * XLEN is the width of the registers in bits and the width of what most instructions use
 * (if instructions operate on non-XLEN sized bits, they usually add an identifier that
 * describes the width to the instruction mnemonic (ex: H, W, D, Q)).
 * @see 2.1 Programmers' Model for Base Integer ISA (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_programmers_model_for_base_integer_isa)
 */
#define XLEN 64

/**
 * Number of harts (what riscv land calls cpu cores).
 */
#define HART_COUNT 2

/**
 * MARK: Vector Config/Notes
 * Extension: Zvl128b
 */

/**
 * ELEN (Element Length) Maximum size in bits of a vector element that any operation can
 * produce or consume.
 * ELEN >= 8, and must be a power of 2. Part of the "V" extension.
 * @see 30.2 Implementation-defined Constant Parameters (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_implementation_defined_constant_parameters)
 */
#define ELEN 64

/**
 * VLEN (Vector Length) is the number of bits in a single vector register. VLEN >= ELEN, and
 * must be a power of 2, and must not be greater than 2^16. Part of the "V" extension.
 * @see 30.2 Implementation-defined Constant Parameters (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_implementation_defined_constant_parameters)
 */
#define VLEN 128
#define VLEN_BYTES 16


#endif // RV64_VIRTUAL_HARDWARE_CONF_H

