
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
    I_MOVE,
    I_MOVE_IMMEDIATE,
    I_MOVE_IMMEDIATE_TO_REGISTER,
    I_MOVE_TO_AX,
    I_MOVE_AX,
    I_MOVE_TO_SEGMENT_REGISTER,
    I_MOVE_SEGMENT_REGISTER,
    // PUSH
    I_PUSH,
    I_PUSH_REGISTER,
    I_PUSH_SEGMENT_REGISTER,
    // POP
    I_POP,
    I_POP_REGISTER,
    I_POP_SEGMENT_REGISTER,
    // XCHG
    I_EXCHANGE,
    I_EXCHANGE_AX,
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

    // MARK: ARITHMETIC
    // ADD
    I_ADD,
    I_ADD_IMMEDIATE,
    I_ADD_IMMEDIATE_TO_AX,
    // ADC
    I_ADC,
    I_ADC_IMMEDIATE,
    I_ADC_IMMEDIATE_TO_AX,
    // INC
    I_INC,
    I_INC_REGISTER,
    I_AAA_ASCII_ADJUST_FOR_ADD,
    I_DAA_DECIMAL_ADJUST_FOR_ADD,
    // SUB
    I_SUB,
    I_SUB_IMMEDIATE,
    I_SUB_IMMEDIATE_FROM_AX,
    // SBB
    I_SBB,
    I_SBB_IMMEDIATE,
    I_SBB_IMMEDIATE_FROM_AX,
    // DEC
    I_DEC_REGISTER_OR_MEMORY,
    I_DEC_REGISTER,
    // NEG
    I_NEGATE_CHANGE_SIGN,
    // CMP
    I_COMPARE,
    I_COMPARE_IMMEDIATE,
    I_COMPARE_IMMEDIATE_WITH_AX,
    // AAS
    I_ASCII_ADJUST_FOR_SUBTRACT,
    // DAS
    I_DECIMAL_ADJUST_FOR_SUBTRACT,
    // MUL
    I_MULTIPLY_UNSIGNED,
    // IMUL
    I_INTEGER_MULTIPLY_SIGNED,
    // AAM
    I_ASCII_ADJUST_FOR_MULTIPLY,
    // DIV
    I_DIVIDE_UNSIGNED,
    // IDIV
    I_INTEGER_DIVIDE_SIGNED,
    // AAD
    I_ASCII_ADJUST_FOR_DIVIDE,
    // CBW
    I_CONVERT_BYTE_TO_WORD,
    // CWD
    I_CONVERT_WORD_TO_DOUBLE_WORD,

    // MARK: LOGIC
    // NOT
    I_NOT,
    // SHL/SAL
    I_SHIFT_LOGICAL_LEFT,
    // SHR
    I_SHIFT_LOGICAL_RIGHT,
    // SAR
    I_SHIFT_ARITHMETIC_RIGHT,
    // ROL
    I_ROTATE_LEFT,
    // ROR
    I_ROTATE_RIGHT,
    // RCL
    I_ROTATE_LEFT_CARRY,
    // RCR
    I_ROTATE_RIGHT_CARRY,
    // AND
    I_AND,
    I_AND_IMMEDIATE,
    I_AND_IMMEDIATE_TO_AX,
    // TEST
    I_TEST_REGISTER_OR_MEMORY_AND_REGISTER,
    I_TEST_IMMEDIATE_DATA_AND_REGISTER_OR_MEMORY,
    I_TEST_IMMEDIATE_DATA_AND_ACCUMULATOR,
    // OR
    I_OR_REGISTER_OR_MEMORY_AND_REGISTER_TO_EITHER,
    I_OR_IMMEDIATE_TO_REGISTER_OR_MEMORY,
    I_OR_IMMEDIATE_TO_ACCUMULATOR,
    // XOR
    I_XOR_REGISTER_OR_MEMORY_AND_REGISTER_TO_EITHER,
    I_XOR_IMMEDIATE_TO_REGISTER_OR_MEMORY,
    I_XOR_IMMEDIATE_TO_ACCUMULATOR,

    // MARK: STRING MANIPULATION
    // REP
    I_REPEAT,
    // MOVS
    I_STRING_MOVE,
    // CMPS
    I_STRING_COMPARE,
    // SCAS
    I_STRING_SCAN,
    // LODS
    I_STRING_LOAD,
    // STDS
    I_STRING_STORE,

    // MARK: CONTROL TRANSFER
    // CALL
    I_CALL_DIRECT_WITHIN_SEGMENT,
    I_CALL_INDIRECT_WITHIN_SEGMENT,
    I_CALL_DIRECT_INTERSEGMENT,
    I_CALL_INDIRECT_INTERSEGMENT,
    // JMP
    I_JUMP_DIRECT_WITHIN_SEGMENT,
    I_JUMP_DIRECT_WITHIN_SEGMENT_SHORT,
    I_JUMP_INDIRECT_WITHIN_SEGMENT,
    I_JUMP_DIRECT_INTERSEGMENT,
    I_JUMP_INDIRECT_INTERSEGMENT,
    // RET
    I_RETURN_WITHIN_SEGMENT,
    I_RETURN_WITHIN_SEGMENT_ADD_IMMEDIATE_TO_SP,
    I_RETURN_INTERSEGMENT,
    I_RETURN_INTERSEGMENT_ADD_IMMEDIATE_TO_SP,
    // JE/JZ
    I_JUMP_ON_EQUAL,
    // JL/JNGE
    I_JUMP_ON_LESS,
    // JLE/JNG
    I_JUMP_ON_LESS_OR_EQUAL,
    // JB/JNAE
    I_JUMP_ON_BELOW,
    // JBE/JNA
    I_JUMP_ON_BELOW_OR_EQUAL,
    // JP/JPE
    I_JUMP_ON_PARITY,
    // JO
    I_JUMP_ON_OVERLFLOW,
    // JS
    I_JUMP_ON_SIGN,
    // JNE/JNZ
    I_JUMP_ON_NOT_EQUAL,
    // JNL/JGE
    I_JUMP_ON_GREATER_OR_EQUAL,
    // JNLE/JG
    I_JUMP_ON_GREATER,
    // JNB/JAE
    I_JUMP_ON_ABOVE_OR_EQUAL,
    // JNBE/JA
    I_JUMP_ON_ABOVE,
    // JNP/JPO
    I_JUMP_ON_NOT_PARITY,
    // JNO
    I_JUMP_ON_NOT_OVERFLOW,
    // JNS
    I_JUMP_ON_NOT_SIGN,
    // LOOP
    I_LOOP,
    // LOOPZ/LOOPE
    I_LOOP_WHILE_EQUAL,
    // LOOPNZ/LOOPNE
    I_LOOP_WHILE_NOT_EQUAL,
    // JCXZ
    I_JUMP_ON_CX_ZERO,
    // INT
    I_INTERRUPT_TYPE_SPECIFIED,
    I_INTERRUPT_TYPE_3,
    // INTO
    I_INTERRUPT_ON_OVERFLOW,
    // IRET
    I_INTERRUPT_RETURN,

    // MARK: PROCESSOR CONTROL
    // CLC
    I_CLEAR_CARRY,
    // CMC
    I_COMPLEMENT_CARRY,
    // STC
    I_SET_CARRY,
    // CLD
    I_CLEAR_DIRECTION,
    // STD
    I_SET_DIRECTION,
    // CLI
    I_CLEAR_INTERRUPT,
    // STI
    I_SET_INTERRUPT,
    // HLT
    I_HALT,
    // WAIT
    I_WAIT,
    // ESC
    I_ESCAPE,
    // LOCK
    I_LOCK,
    // SEGMENT
    I_SEGMENT,
} instruction_tag_t;

/**
 * Lookup table for machine instructions to their associated assembly instruction mnemonics.
 * TODO: maybe have a function that returns access into this and have a #define that allows
 * compiling this out to reduce binary size if needed. the function would just return "?" for
 * each instruction if this is compiled out.
 * WARNING: Parallal array with "instruction_tag_t".
 */
static char instruction_tag_mnemonic[][8] = {
    "invalid",  // I_INVALID
    "mov",      // I_MOVE
    "mov",      // I_MOVE_IMMEDIATE
    "mov",      // I_MOVE_IMMEDIATE_TO_REGISTER
    "mov",      // I_MOVE_TO_AX
    "mov",      // I_MOVE_AX
    "mov",      // I_MOVE_TO_SEGMENT_REGISTER
    "mov",      // I_MOVE_SEGMENT_REGISTER
    "push",     // I_PUSH
    "push",     // I_PUSH_REGISTER
    "push",     // I_PUSH_SEGMENT_REGISTER
    "pop",      // I_POP
    "pop",      // I_POP_REGISTER
    "pop",      // I_POP_SEGMENT_REGISTER
    "xchg",     // I_EXCHANGE
    "xchg",     // I_EXCHANGE_AX
    "in",       // I_INPUT_FROM_FIXED_PORT
    "in",       // I_INPUT_FROM_VARIABLE_PORT
    "out",      // I_OUTPUT_TO_FIXED_PORT
    "out",      // I_OUTPUT_TO_VARIABLE_PORT
    "xlat",     // I_TRANSLATE_BYTE_TO_AL
    "lea",      // I_LOAD_EA_TO_REGISTER
    "lds",      // I_LOAD_POINTER_TO_DS
    "les",
    "lahf",
    "sahf",
    "pushf",
    "popf",
    "add",      // I_ADD
    "add",      // I_ADD_IMMEDIATE
    "add",      // I_ADD_IMMEDIATE_TO_AX
    "adc",
    "adc",
    "adc",
    "inc",
    "inc",
    "aaa",
    "daa",
    "sub",      // I_SUB
    "sub",
    "sub",
    "sbb",
    "sbb",
    "sbb",
    "dec",
    "dec",
    "neg",
    "cmp",      // I_COMPARE
    "cmp",
    "cmp",
    "aas",
    "das",
    "mul",
    "imul",
    "aam",
    "div",
    "idiv",
    "add",
    "cbw",
    "cwd",
    "not",
    "shl",
    "shr",
    "sar",
    "rol",
    "ror",
    "rcl",
    "rcr",
    "and",
    "and",
    "and",
    "test",
    "test",
    "test",
    "or",
    "or",
    "or",
    "xor",
    "xor",
    "xor",
    "rep",
    "movs",
    "cmps",
    "scas",
    "lods",
    "stds",
    "call",
    "call",
    "call",
    "call",
    "jmp",
    "jmp",
    "jmp",
    "jmp",
    "jmp",
    "ret",
    "ret",
    "ret",
    "ret",
    "je",
    "jl",
    "jle",
    "jb",
    "jbe",
    "jp",
    "jo",
    "js",
    "jne",
    "jnl",
    "jnle",
    "jnb",
    "jnbe",
    "jnp",
    "jno",
    "jns",
    "loop",
    "loopz",
    "loopnz",
    "jcxz",
    "int",
    "into",
    "iret",
    "clc",
    "cmc",
    "stc",
    "cld",
    "std",
    "cli",
    "sti",
    "hlt",
    "wait",
    "esc",
    "lock",
    "segment",
};

/**
 * 1 = the REG field in the second byte identifies the destination operand
 * 0 = the REG field identifies the source operand
 */
typedef enum {
    DIR_REG_SOURCE,
    DIR_REG_DEST,
} direction_t;

/**
 * 
 * 8086: Table 4.7. Single Bit Field Encoding (page 160)
 */
typedef enum {
    WIDE_BYTE = 0, // Instruction operates on byte data (8 bit)
    WIDE_WORD = 1, // Instruction operates on word data (16 bit)
    // May expand the scope of this to i386 or x64
    // WIDE_DWORD = 2, // Instruction operates on double word data (32 bit)
    // WIDE_QWORD = 3, // Instruction operates on quad word data (64 bit)
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

#define REG_DIRECT_ACCESS (0b110) // 6
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

#endif // INSTRUCTIONS_H
