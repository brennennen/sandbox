
#ifndef INSTRUCTIONS_A64_H
#define INSTRUCTIONS_A64_H

#include <stdint.h>
#include <inttypes.h>

#include "shared/include/binary_utilities.h"

// MARK: Instruction Groups
/**
 * All instructions are grouped into the categories below.
 * https://developer.arm.com/documentation/ddi0602/2025-03/Index-by-Encoding?lang=en
 */
typedef enum ENUM_PACK_ATTRIBUTE {
    IG_INVALID,
    IG_RESERVED,
    IG_SME,
    IG_SVE,
    IG_UNALLOCATED,
    IG_DATA_PROCESSING_IMMEDIATE,
    IG_BRANCHES_EXCEPTIONS_SYSTEM,
    IG_DATA_PROCESSING_REGISTER,
    IG_DATA_PROCESSING_SCALAR_FP_AND_SIMD,
    IG_LOADS_AND_STORES
} instruction_group_a64_t;

// MARK: Instructions
/**
 * All Instructions for the 8086 processor as defined in the "8086 family users manual".
 * See Table 4-12. "8086 Instruction Encoding" (~page 164).
 */
typedef enum ENUM_PACK_ATTRIBUTE {
    I_INVALID,
    // Reserved
    // ...
    // SME Encodings
    // ...
    // SVE Encodings
    // ...
    // MARK: Data Processing -- Immediate
    //  Data-Processing (1 source immediate)
    //  PC-rel. addressing
    //  Add/subtract (immediate)
    I_ADD_IMMEDIATE,
    I_ADDS_IMMEDIATE,
    I_SUB_IMMEDIATE,
    I_SUBS_IMMEDIATE,
    //  Add/subtract (immediate, with tags)
    I_ADDG,
    I_SUBG,
    //  Min/max (immediate)
    //  Logical (immediate)
    //  Move wide (immediate)
    //  Bitfield
    //  Extract

    // Branches, Exception Generating, and System Instructions
    // ...
    // Data Processing -- Register
    // ...
    // Data Processing -- Scalar Floating-Point and Advanced SMD
    // ...
    // Loads and Stores
    // ...
} instruction_tag_a64_t;

#endif // INSTRUCTIONS_A64_H
