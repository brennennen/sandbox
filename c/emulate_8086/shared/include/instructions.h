
#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>
#include <inttypes.h>

#include "shared/include/binary_utilities.h"

// MARK: Instruction Tags
/**
 * All Instructions for the 8086 processor as defined in the "8086 family users manual".
 * See Table 4-12. "8086 Instruction Encoding" (~page 164).
 * 
 * NOTE: Parallel array with "opcodes" and "instruction_metadata"! If you update this 
 * enum, you need to also update both of these arrays too!
 */
typedef enum ENUM_PACK_ATTRIBUTE {
    I_INVALID = 0,
    // MOV
    I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY,
    I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY,
    I_MOVE_IMMEDIATE_TO_REGISTER,
    I_MOVE_MEMORY_TO_ACCUMULATOR,
    // NOT IMPLEMENTED
    I_MOVE_ACCUMULATOR_TO_MEMORY,
    I_MOVE_REGISTER_OR_MEMORY_TO_SEGMENT_REGISTER,
    I_MOVE_SEGMENT_REGISTER_TO_REGISTER_OR_MEMORY,
    // // PUSH
    // I_PUSH_REGISTER_OR_MEMORY,
    // I_PUSH_REGISTER,
    // I_PUSH_SEGMENT_REGISTER,
    // // XCHNG
    // I_XCHNG_REGISTER_OR_MEMORY_WITH_REGISTER,
    // I_XCHNG_REGISTER_WITH_ACCUMULATOR,
    // IN
    // OUT
    // ARITHMETIC
    // ADD
    // ADC
    // INC
    // SUB
    // SBB
    // DEC
    // NEG
    // CMP
} instruction_tag_t;

// MARK: opcodes
typedef struct opcode {
    uint8_t opcode;        // ex: 0b100010XX for MOV (rightmost 2 bits are flags and not part of the opcode)
    uint8_t opcode_mask;   // ex: 0b11111100
} opcode_t;

/**
 * NOTE: Parallel array with "instruction_tag_t" enum!
 */
static opcode_t opcodes[] = {
    {   // I_INVALID
        .opcode = 0b00000000,
        .opcode_mask = 0b11111111,
    },
    {   // I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY
        .opcode = 0b10001000,
        .opcode_mask = 0b11111100,
    },
    {   // I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY
        .opcode = 0b11000110,
        .opcode_mask = 0b11111110,
    },
    {   // I_MOVE_IMMEDIATE_TO_REGISTER
        .opcode = 0b10110000,      
        .opcode_mask = 0b11110000,
    },
    {   // I_MOVE_MEMORY_TO_ACCUMULATOR
        .opcode = 0b10100000, 
        .opcode_mask = 0b11111110,
    },
    {   // I_MOVE_ACCUMULATOR_TO_MEMORY
        .opcode = 0b10100010, 
        .opcode_mask = 0b11111110,
    },
    {   // I_MOVE_REGISTER_OR_MEMORY_TO_SEGMENT_REGISTER
        .opcode = 0b10001110, 
        .opcode_mask = 0b11111111,
    },
    {   // I_MOVE_SEGMENT_REGISTER_TO_REGISTER_OR_MEMORY
        .opcode = 0b10001100, 
        .opcode_mask = 0b11111111,
    },
    // PUSH
    // {   // I_PUSH_REGISTER_OR_MEMORY
    //     .opcode = 0b11111111, 
    //     .opcode_mask = 0b11111111,
    // },
    // {   // I_PUSH_REGISTER
    //     .opcode = 0b01010000, 
    //     .opcode_mask = 0b11111000,
    // },
    // {   // I_PUSH_SEGMENT_REGISTER
    //     .opcode = 0b00000111, 
    //     .opcode_mask = 0b11100111,
    // },
    // // XCHNG
    // {   // I_XCHNG_REGISTER_OR_MEMORY_WITH_REGISTER
    //     .opcode = 0b10000110, 
    //     .opcode_mask = 0b11111110,
    // },
    // {   // I_XCHNG_REGISTER_WITH_ACCUMULATOR
    //     .opcode = 0b10010000,
    //     .opcode_mask = 0b11111000,
    // },
};

typedef struct instruction_metadata {
    char mnemonic[16];
    char name[256];
    char description[256];
} instruction_metadata_t;

/**
 * 
 * NOTE: Parallel array with "instruction_tag_t" enum!
 */
static instruction_metadata_t instruction_metadata[] = {
    {
        .mnemonic = "inv", 
        .name = "Invalid", 
        .description = "Invalid opcode",
    },
    // MOV
    {   // I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY
        .mnemonic = "mov", 
        .name = "Move", 
        .description = "Register/memory to/from register",
    },
    {   // I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY
        .mnemonic = "mov", 
        .name = "Move", 
        .description = "Immediate to register/memory",
    },
    {   // I_MOVE_IMMEDIATE_TO_REGISTER
        .mnemonic = "mov", 
        .name = "Move", 
        .description = "Immediate to register",
    },
    { 
        .mnemonic = "mov", 
        .name = "Move", 
        .description = "Memory to accumulator",
    },
    {
        .mnemonic = "mov", 
        .name = "Move", 
        .description = "Accumulator to memory",
    },
    {
        .mnemonic = "mov", 
        .name = "Move", 
        .description = "Register/memory to segment register",
    },
    {
        .mnemonic = "mov", 
        .name = "Move", 
        .description = "Segment register to register/memory",
    },
    // PUSH
    {
        .mnemonic = "push", 
        .name = "Push", 
        .description = "Register/memory",
    },
    {
        .mnemonic = "push", 
        .name = "Push", 
        .description = "Register/memory",
    },
    {
        .mnemonic = "push", 
        .name = "Push", 
        .description = "Segment register",
    },
    // XCHNG
    {
        .mnemonic = "xchg", 
        .name = "Exchange", 
        .description = "Register/memory with register",
    },
    {
        .mnemonic = "xchg", 
        .name = "Exchange", 
        .description = "Register with accumulator",
    },
    // IN
    {
        .mnemonic = "in", 
        .name = "Input From", 
        .description = "Fixed port",
    },
    {
        .mnemonic = "in", 
        .name = "Input From", 
        .description = "Variable port",
    }
};

/**
 * 1 = the REG field in the second byte identifies the destination operand
 * 0 = the REG field identifies the source operand
 */
typedef enum {
    DIR_TO_REGISTER,
    DIR_FROM_REGISTER,
} direction_t;

/**
 * 
 * Table 4.7. Single Bit Field Encoding (page 160)
 */
typedef enum {
    WIDE_BYTE = 0, // Instruction operates on byte data
    WIDE_WORD = 1, // Instruction operates on word data
} wide_t;

/**
 * Indicates whether one of the operands is in memory or whether both operands
 * are registers.
 */
typedef enum {
    MOD_MEMORY = 0,
    MOD_MEMORY_8BIT_DISPLACEMENT = 1,
    MOD_MEMORY_16BIT_DISPLACEMENT = 2,
    MOD_REGISTER = 3,
} mod_t;

static char mod_strings[][64] = {
    "Memory Mode, no displacement follows*",
    "Memory Mode, 8-bit displacement follows",
    "Memory Mode, 16-bit displacement follows",
    "Register Mode (no displacement)",
};

typedef enum {
    REG_AL_AX = 0,
    REG_CL_CX = 1,
    REG_DL_DX = 2,
    REG_BL_BX = 3,
    REG_AH_SP = 4,
    REG_CH_BP = 5,
    REG_DH_SI = 6,
    REG_BH_DI = 7,
} reg_t;



typedef enum {
    REGB_AL = 0,
    REGB_CL = 1,
    REGB_DL = 2,
    REGB_BL = 3,
    REGB_AH = 4,
    REGB_CH = 5,
    REGB_DH = 6,
    REGB_BH = 7,
} reg_byte_t;

static char regb_strings[][3] = {
    "al",
    "cl",
    "dl",
    "bl",
    "ah",
    "ch",
    "dh",
    "bh",
};

typedef enum {
    REGW_AX = 0,
    REGW_CX = 1,
    REGW_DX = 2,
    REGW_BX = 3,
    REGW_SP = 4,
    REGW_BP = 5,
    REGW_SI = 6,
    REGW_DI = 7,
} reg_wide_t;

static char regw_strings[][3] = {
    "ax",
    "cx",
    "dx",
    "bx",
    "sp",
    "bp",
    "si",
    "di",
};

typedef enum {
    RM_BX_SI = 0,
    RM_BX_DI = 1,
    RM_BP_SI = 2,
    RM_BP_DI = 3,
    RM_SI,
    RM_DI,
    RM_BP,
    RM_BX,
} rm_effective_address_calc_t;


// MARK: MOV
// 7 different mov encodings
typedef struct {
    uint8_t fields1;
    uint8_t fields2;
    uint16_t displacement;
} move_register_or_memory_to_or_from_register_or_memory_t;
typedef struct {
    uint8_t fields1;
    uint8_t fields2;
    uint16_t displacement;
    uint16_t immediate;
} move_immediate_to_register_or_memory_t;
typedef struct {
    uint8_t fields1;
    uint16_t immediate;
} move_immediate_to_register_t;
typedef struct {
    uint8_t fields1;
    uint16_t address;
} move_memory_to_accumulator_t;
typedef struct {
    uint8_t fields1;
    uint16_t address;
} move_accumulator_to_memory_t;
typedef struct {
    uint8_t fields1;
    uint8_t fields2;
    uint16_t displacement;
} move_register_or_memory_to_segment_register_t;
typedef struct {
    uint8_t fields1;
    uint8_t fields2;
    uint16_t displacement;
} move_segment_register_to_register_or_memory_t;

// MARK: PUSH
typedef struct {
    uint8_t fields1;
    uint8_t fields2;
    uint16_t displacement;
} push_register_or_memory_t;
typedef struct {
    uint8_t fields1;
} push_register_t;
typedef struct {
    uint8_t fields1;
} push_segment_register_t;

// MARK: POP
// ...

// MARK: Instruction

/**
 * Each member should be 6 bytes max
 */
typedef union instruction_data {
    // MOV
    move_register_or_memory_to_or_from_register_or_memory_t move_register_or_memory_to_or_from_register_or_memory;
    move_immediate_to_register_or_memory_t move_immediate_to_register_or_memory;
    move_immediate_to_register_t move_immediate_to_register;
    move_memory_to_accumulator_t move_memory_to_accumulator;
    move_accumulator_to_memory_t move_accumulator_to_memory;
    move_register_or_memory_to_segment_register_t move_register_or_memory_to_segment_register;
    move_segment_register_to_register_or_memory_t move_segment_register_to_register_or_memory;
    // PUSH
    push_register_or_memory_t push_register_or_memory;
    push_register_t push_register;
    push_segment_register_t push_segment_register;
    // POP
    // ...
} instruction_data_t;

typedef struct PACK_ATTRIBUTE {
    instruction_tag_t tag; // instruction_tag_t packed.
    instruction_data_t data;
    
    // rather than the tagged union approach, it's probably better to just store the data
    // and re-calc any field needed on the fly. math is cheap.
    //uint8_t byte1;
    //uint8_t byte2;
    //uint16_t data;
    //uint16_t displacement;
} instruction_t;

#endif // INSTRUCTIONS_H
