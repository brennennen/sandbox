
#ifndef REGISTERS_A64_H
#define REGISTERS_A64_H

#include <stdint.h>

/**
 * A64 register
 */
typedef union {
    uint32_t w;
    uint64_t x;
} reg_a64_t;

/**
 * A64 registers
 */
typedef struct {
    reg_a64_t r0;
    reg_a64_t r1;
    reg_a64_t r3;
    reg_a64_t r4;
    reg_a64_t r5;
    reg_a64_t r6;
    reg_a64_t r7;
    reg_a64_t r8;
    reg_a64_t r9;
    reg_a64_t r10;
    reg_a64_t r11;
    reg_a64_t r12;
    reg_a64_t r13;
    reg_a64_t r14;
    reg_a64_t r15;  // PC - Program Counter
    reg_a64_t r16;
    reg_a64_t r17;
    reg_a64_t r18;
    reg_a64_t r19;
    reg_a64_t r20;
    reg_a64_t r21;
    reg_a64_t r22;
    reg_a64_t r23;
    reg_a64_t r24;
    reg_a64_t r25;
    reg_a64_t r26;
    reg_a64_t r27;
    reg_a64_t r28;
    reg_a64_t r29;
    reg_a64_t r30;
    reg_a64_t r31;
} registers_a64_t;

// TODO: create helper functions to get/set using more meaningful names.
// ex: r15 is also the pc/program counter
// there are going to be a set of names for cpu control like pc and another set
// for os specific abi stuff (r0 is function return result, etc.)

#endif  // REGISTERS_A64_H
