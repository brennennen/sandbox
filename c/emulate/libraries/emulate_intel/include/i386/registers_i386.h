#ifndef REGISTERS_i386_H
#define REGISTERS_i386_H

#include <stdint.h>

/**
 *
 * See Figure 2-10.
 * https://stackoverflow.com/questions/9130349/how-many-registers-are-there-in-8086-8088
 */
typedef struct {
    // 16 bits wide, can be divide into low and high 8 bit sections
    uint32_t eax;  // Accumulator
    uint32_t ebx;  // Base
    uint32_t ecx;  // Count
    uint32_t edx;  // Data

    // 16 bits wide, can not be divide
    uint32_t esp;  // Stack Pointer
    uint32_t ebp;  // Base Pointer
    uint32_t esi;  // Source Index
    uint32_t edi;  // Destination Index

    // Segment Registers
    uint32_t cs;  // Code Segment
    uint32_t ds;  // Data Segment
    uint32_t es;  // Stack Segment
    uint32_t ss;  // Extra Segment

    // TODO: read data sheet for other registers?

    // special purpose
    uint32_t ip;  // Instruction Pointer

    // flag register
    uint32_t flags;
} registers_i386_t;

#endif
