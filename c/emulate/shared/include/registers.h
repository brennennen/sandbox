/**
 *
 *
 */
#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>

/* */
/**
 *
 *
 * Flags, Figure 2-10 (page ~24)
 * Arithmetic Insrtuctions and Flags (page ~49)
 */
#define FLAG_CF_MASK 0b0000000000000001 // Carry
#define FLAG_PF_MASK 0b0000000000000100 // Parity
#define FLAG_AF_MASK 0b0000000000010000 // Auxiliary Carry
#define FLAG_ZF_MASK 0b0000000001000000 // Zero
#define FLAG_SF_MASK 0b0000000010000000 // Sign
#define FLAG_TF_MASK 0b0000000100000000 // Trap
#define FLAG_IF_MASK 0b0000001000000000 // Interrupt-Enable
#define FLAG_DF_MASK 0b0000010000000000 // Direction
#define FLAG_OF_MASK 0b0000100000000000 // Overflow

#define FLAG_CF_BIT_OFFSET 0
#define FLAG_PF_BIT_OFFSET 2
#define FLAG_AF_BIT_OFFSET 4
#define FLAG_ZF_BIT_OFFSET 6
#define FLAG_SF_BIT_OFFSET 7
#define FLAG_TF_BIT_OFFSET 8
#define FLAG_IF_BIT_OFFSET 9
#define FLAG_DF_BIT_OFFSET 10
#define FLAG_OF_BIT_OFFSET 11


/**
 *
 * See Figure 2-10.
 * https://stackoverflow.com/questions/9130349/how-many-registers-are-there-in-8086-8088
 */
typedef struct {
    // 16 bits wide, can be divide into low and high 8 bit sections
    uint16_t ax; // Accumulator
    uint16_t bx; // Base
    uint16_t cx; // Count
    uint16_t dx; // Data

    // 16 bits wide, can not be divide
    uint16_t sp; // Stack Pointer
    uint16_t bp; // Base Pointer
    uint16_t si; // Source Index
    uint16_t di; // Destination Index

    // Segment Registers
    uint16_t cs; // Code Segment
    uint16_t ds; // Data Segment
    uint16_t es; // Stack Segment
    uint16_t ss; // Extra Segment

    // special purpose
    uint16_t ip; // Instruction Pointer

    // flag register
    uint16_t flags;
} registers_t;



#endif // REGISTERS_H
