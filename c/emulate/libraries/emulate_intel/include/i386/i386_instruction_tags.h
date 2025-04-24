#ifndef i386_INSTRUCTION_TAGS_H
#define i386_INSTRUCTION_TAGS_H

#include "binary_utilities.h"


// MARK: Instruction Tags
/**
 * All instruction tags for the i386 processor (Intel 80386 Programmer's Reference Manual,
 * 17.2.2.11, page 262).
 *
 * 8086 differences:
 * * In the reference manual, they stopped referencing the "w" or "d" bits that were part of
 * the first opcode byte in the 8086 and just call each separate full byte a different opcode.
 */
typedef enum ENUM_PACK_ATTRIBUTE {
    I_i386_INVALID = 0,
    // ...

    // MARK: AND
    I_i386_AND_AX, // AND opcodes 1 - 3
    I_i386_AND_IMMEDIATE, // AND opcodes 4 - 8
    I_i386_AND, // AND opcodes 9 - 14

    // ...
} i386_instruction_tag_t;

#endif // i386_INSTRUCTION_TAGS_H
