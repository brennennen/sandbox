
#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>
#include <inttypes.h>

#include "shared/include/binary_utilities.h"

// MARK: Instruction Tags
/**
 * All Instructions for the 8086 processor as defined in the "8086 family users manual".
 * See Table 4-12. "8086 Instruction Encoding" (~page 164).
 */
typedef enum ENUM_PACK_ATTRIBUTE {
    I_INVALID = 0,
    // MOV
    I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY,
    I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY,
    I_MOVE_IMMEDIATE_TO_REGISTER,
    I_MOVE_MEMORY_TO_ACCUMULATOR,
    I_MOVE_ACCUMULATOR_TO_MEMORY,
    I_MOVE_REGISTER_OR_MEMORY_TO_SEGMENT_REGISTER,
    I_MOVE_SEGMENT_REGISTER_TO_REGISTER_OR_MEMORY,
    // PUSH
    I_PUSH_REGISTER_OR_MEMORY,
    I_PUSH_REGISTER,
    I_PUSH_SEGMENT_REGISTER,
    // POP
    I_POP_REGISTER_OR_MEMORY,
    I_POP_REGISTER,
    I_POP_SEGMENT_REGISTER,
    // XCHNG
    I_EXCHANGE_REGISTER_OR_MEMORY_WITH_REGISTER,
    I_EXCHANGE_REGISTER_WITH_ACCUMULATOR,
    // IN
    I_INPUT_FROM_FIXED_PORT,
    I_INPUT_FROM_VARIABLE_PORT,
    // OUT
    I_OUTPUT_TO_FIXED_PORT,
    I_OUTPUT_TO_VARIABLE_PORT,
    // XLAT
    I_TRANSLATE_BYTE_TO_AL,
    // LEA
    I_LOAD_EA_TO_REGISTER,
    // LDS
    I_LOAD_POINTER_TO_DS,
    // LES
    I_LOAD_POINTER_TO_ES,
    // LAHF
    I_LOAD_AH_WITH_FLAGS,
    // SAHF
    I_STORE_AH_INTO_FLAGS,
    // PUSHF
    I_PUSH_FLAGS,
    // POPF
    I_POP_FLAGS,
    // ADD
    I_ADD_REGISTER_OR_MEMORY_WITH_REGISTER_TO_EITHER,
    I_ADD_IMMEDIATE_TO_REGISTER_OR_MEMORY,
    I_ADD_IMMEDIATE_TO_ACCUMULATOR,
    // ADC
    I_ADC_REGISTER_OR_MEMORY_WITH_REGISTER_TO_EITHER,
    I_ADC_IMMEDIATE_TO_REGISTER_OR_MEMORY,
    I_ADC_IMMEDIATE_TO_ACCUMULATOR,
    // INC
    I_INC_REGISTER_OR_MEMORY,
    I_INC_REGISTER,
    I_AAA_ASCII_ADJUST_FOR_ADD,
    I_DAA_DECIMAL_ADJUST_FOR_ADD,
    // SUB
    I_SUB_REGISTER_OR_MEMORY_AND_REGISTER_TO_EITHER,
    I_SUB_IMMEDIATE_FROM_REGISTER_OR_MEMORY,
    I_SUB_IMMEDIATE_FROM_ACCUMULATOR,
    // SBB
    I_SBB_REGISTER_OR_MEMORY_AND_REGISTER_TO_EITHER,
    I_SBB_IMMEDIATE_FROM_REGISTER_OR_MEMORY,
    I_SBB_IMMEDIATE_FROM_ACCUMULATOR,
    // DEC
    I_DEC_REGISTER_OR_MEMORY,
    I_DEC_REGISTER,
    // NEG
    I_NEGATE_CHANGE_SIGN,
    // CMP
    I_COMPARE_REGISTER_OR_MEMORY_AND_REGISTER,
    I_COMPARE_IMMEDIATE_WITH_REGISTER_OR_MEMORY,
    I_COMPARE_IMMEDIATE_WITH_ACCUMULATOR,
    // AAS
    I_ASCII_ADJUST_FOR_SUBTRACT,
    // DAS
    I_DECIMAL_ADJUST_FOR_SUBTRACT,
    // MUL
    I_MULTIPLY_UNSIGNED,
    // IMUL
    I_INTEGER_MULTIPLY_SIGNED,
    // AAM
    // DIV
    // IDIV
    // AAD
    // CBW
    // CWD
    // LOGIC
    // NOT
    // SHL/SAL
    // SHR
    // SAR
    // ROL
    // ROR
    // RCL
    // RCR
    // AND
    // TEST
    // OR
    // XOR
    // STRING MANIPULATION
    // REP
    // MOVS
    // CMPS
    // SCAS
    // LODS
    // STDS
    // CALL
    // JMP
    // RET
    // JE/JZ
    // JL/JNGE
    // JLE/JNG
    // JB/JNAE
    // JBE/JNA
    // JP/JPE
    // JO
    // JS
    // JNE/JNZ
    // JNL/JGE
    // JNLE/JG
    // JNB/JAE
    // JNBE/JA
    // JNP/JPO
    // JNO
    // JNS
    // LOOP
    // LOOPZ/LOOPE
    // LOOPNZ/LOOPNE
    // JCXZ
    // INT
    // INTO
    // IRET
    // CLC
    // CMC
    // STC
    // CLD
    // STD
    // CLI
    // STI
    // HLT
    // WAIT
    // ESC
    // LOCK
    // SEGMENT

} instruction_tag_t;

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
