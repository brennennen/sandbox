
#ifndef REGISTERS_RV64_H
#define REGISTERS_RV64_H

#include <stdint.h>



/**
 * The RISCV spec doesn't specify register names, they just use x0 - x31 in the
 * spec and allow hardware vendors to decide on ABI details that define which
 * register is commonly used for what. The names below are what the GNU folks
 * are using and probably the most common convention.
 */

#define RV64_REG_ZERO 0
#define RV64_REG_RA 1
#define RV64_REG_SP 2
#define RV64_REG_GP 3
#define RV64_REG_TP 4
#define RV64_REG_T0 5
#define RV64_REG_T1 6
#define RV64_REG_T2 7
#define RV64_REG_FP 8
#define RV64_REG_S1 9
#define RV64_REG_A0 10
#define RV64_REG_A1 11
#define RV64_REG_A2 12
#define RV64_REG_A3 13
#define RV64_REG_A4 14
#define RV64_REG_A5 15
#define RV64_REG_A6 16
#define RV64_REG_A7 17
#define RV64_REG_S2 18
#define RV64_REG_S3 19
#define RV64_REG_S4 20
#define RV64_REG_S5 21
#define RV64_REG_S6 22
#define RV64_REG_S7 23
#define RV64_REG_S8 24
#define RV64_REG_S9 25
#define RV64_REG_S10 26
#define RV64_REG_S11 27
#define RV64_REG_T3 28
#define RV64_REG_T4 29
#define RV64_REG_T5 30
#define RV64_REG_T6 31

/**
 *
 * @see 2.1 Programmers' Model for Base Integer ISA (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_programmers_model_for_base_integer_isa)
 */
typedef struct {
    uint64_t regs[32];
    uint32_t pc;

    // TODO: CSR registers? or put those in a different struct?

    // RV64V
    uint64_t vregs[64]; // faking 128 bit wide registers
    uint8_t vta; // @see 30.3.4.3 Vector Tail Agnostic and Vector Mask Agnostic `vta` and `vma` (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#sec-agnostic)
    uint8_t vma; // @see 30.3.4.3 Vector Tail Agnostic and Vector Mask Agnostic `vta` and `vma` (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#sec-agnostic)
    uint8_t vill; // @see 30.3.4.4 Vector Type Illegal (vill) (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_vector_type_illegal_vill)
    uint16_t vl; // @see 30.3.5 Vector Length (vl) Register (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_vector_length_vl_register)
    uint64_t vlenb;
    uint64_t vstart;
    uint64_t vxrm;
    uint8_t vxsat;
    uint64_t vcsr;
    // vtype, sew, lmul, vlmax?



} registers_rv64_t;

// TODO: provide accessor functions for aliased names (args = x0, x1, etc. temp/local
// vars = t0, t1, etc.)

// TODO: vector registers?

#endif // REGISTERS_RV64I_H
