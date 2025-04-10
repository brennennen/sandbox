/**
 * Instruction tag decoding for 8086 instructions. For most instructions, this involves looking
 * at the first chunk of the first byte and matching an opcode to a tag. For some instructions
 * opcodes are shared and you need to look at a secondary opcode that is in the second byte
 * to determine the tag.
 *
 * The order machine instruction decoding matches the specification to make it easier
 * The order of the case statements below matches the order machine instructions are defined
 * int the 8086 family user's manual table 4-12 (8086 Family Users Manual, page 4-22,
 * pdf page ~164).
 *
 * Some assembly instructions map to multiple machine instructions. For example the "MOV"
 * assembly instruction maps to 7 different machine instructions. Throughout this file, the
 * 7 different moves are labeled as "MOV 1", "MOV 2", etc.
 */


#include <stdint.h>

#include "shared/include/instructions.h"

#include "libraries/emulate8086/include/decode_tag.h"

/**
 * Decodes the instruction tag by reading the opcode and sub opcode from the first
 * 2 bytes of an instruction.
 *
 * @param byte1 First byte in the instruction, usually where to read the opcode from.
 * @param byte2 Second byte in the instruction, usually where to read the sub-opcode from if present.
 * @return Instruction tag associated with the opcode/sub-opcode or I_INVALID if no matches.
 */
instruction_tag_t dcd_decode_tag(uint8_t byte1, uint8_t byte2) {
    switch(byte1) {
        // MARK: MOV
        // MOV 1 - 0b100010xx
        case 0b10001000:
        case 0b10001001:
        case 0b10001010:
        case 0b10001011:
            return I_MOVE;
        // MOV 2 - 0b100010xx
        case 0b11000110:
        case 0b11000111:
            return I_MOVE_IMMEDIATE;
        // MOV 3 - 0b1011xxxx
        case 0b10110000:
        case 0b10110001:
        case 0b10110010:
        case 0b10110011:
        case 0b10110100:
        case 0b10110101:
        case 0b10110110:
        case 0b10110111:
        case 0b10111000:
        case 0b10111001:
        case 0b10111010:
        case 0b10111011:
        case 0b10111100:
        case 0b10111101:
        case 0b10111110:
        case 0b10111111:
            return I_MOVE_IMMEDIATE_TO_REGISTER;
        // MOV 4 - 0b1010000x
        case 0b10100000:
        case 0b10100001:
            return I_MOVE_TO_AX;
        // MOV 5 - 0b1010001x
        case 0b10100010:
        case 0b10100011:
            return I_MOVE_AX;
        // MOV 6 - 0b10001110
        case 0b10001110:
            return I_MOVE_TO_SEGMENT_REGISTER;
        // MOV 7 - 0b10001100
        case 0b10001100:
            return I_MOVE_SEGMENT_REGISTER;

        // MARK: COMMON 0b11111111
        case 0b11111111: {
            uint8_t sub_opcode = byte2 & 0b00111000;
            switch(sub_opcode) {
                // PUSH 1
                case 0b00110000:
                    return I_PUSH;

                // INC 1
                case 0b00000000:
                    return I_INC;

                // DEC 1
                case 0b00001000:
                    return I_DEC_REGISTER_OR_MEMORY;

                // CALL 2
                case 0b00010000:
                    return I_CALL_INDIRECT_WITHIN_SEGMENT;
                // CALL 4
                case 0b00011000:
                    return I_CALL_INDIRECT_INTERSEGMENT;

                // JMP 3
                case 0b00100000:
                    return I_JUMP_INDIRECT_WITHIN_SEGMENT;
                // JMP 5
                case 0b00101000:
                    return I_JUMP_INDIRECT_INTERSEGMENT;
                default:
                    break;
            };
            break;
        }

        // MARK: COMMON 0b11111110
        case 0b11111110: {
            uint8_t sub_opcode = byte2 & 0b00111000;
            switch(sub_opcode) {
                // INC 1
                case 0b00000000:
                    return I_INC;

                // DEC 1
                case 0b00001000:
                    return I_DEC_REGISTER_OR_MEMORY;

                default:
                    break;
            };
            break;
        }

        // MARK: COMMON 0b10001111
        case 0b10001111: {
            uint8_t sub_opcode = byte2 & 0b00111000;
            switch(sub_opcode) {
                case 0b00000000:
                    return I_POP;
                default:
                    break;
            };
            break;
        }

        // MARK: PUSH
        // PUSH 1 - I_PUSH - handled by "COMMON 0b11111111"
        // PUSH 2 - 0b01010xxx
        case 0b01010000:
        case 0b01010001:
        case 0b01010010:
        case 0b01010011:
        case 0b01010100:
        case 0b01010101:
        case 0b01010110:
        case 0b01010111:
            return I_PUSH_REGISTER;
        // TODO: look into segment registers. some of the reg range overlaps. guessing there is something reducing segment register reg values
        // PUSH 3 - 0b000xxx110
        // case 0b00000110:
        // case 0b00001110:
        // case 0b00010110:
        // case 0b00011110:
        // case 0b00100110:
        // case 0b00101110:
        // case 0b00110110:
        // case 0b00111110:
        //     return I_PUSH_SEGMENT_REGISTER;

        // MARK: POP
        // POP 1 - I_POP - handled by "COMMON 0b11111111"
        // POP 2 - 0b01011xxx
        case 0b01011000:
        case 0b01011001:
        case 0b01011010:
        case 0b01011011:
        case 0b01011100:
        case 0b01011101:
        case 0b01011110:
        case 0b01011111:
            return I_POP_REGISTER;
        // TODO: look into segment registers.
        // POP 3 - 0b000xxx111
        // case 0b000000111:
        // case 0b000001111:
        // case 0b000010111:
        // case 0b000011111:
        // case 0b000100111:
        // case 0b000101111:
        // case 0b000110111:
        // case 0b000111111:
        //     return I_POP_SEGMENT_REGISTER;

        // MARK: XCHG
        // XCHG 1 - 0b1000011x
        case 0b10000110:
        case 0b10000111:
            return I_EXCHANGE;
        // XCHG 2 - 0b10010xxx
        case 0b10010000:
        case 0b10010001:
        case 0b10010010:
        case 0b10010011:
        case 0b10010100:
        case 0b10010101:
        case 0b10010110:
        case 0b10010111:
            return I_EXCHANGE_AX;

        // MARK: IN
        // IN 1 - 0b1110010x
        case 0b11100100:
        case 0b11100101:
            return I_INPUT_FROM_FIXED_PORT;
        // IN 2 - 0b1110110x
        case 0b11101100:
        case 0b11101101:
            return I_INPUT_FROM_VARIABLE_PORT;

        // MARK: OUT
        // OUT 1 - 0b1110011x
        case 0b11100110:
        case 0b11100111:
            return I_OUTPUT_TO_FIXED_PORT;
        // OUT 2 - 0b1110110x
        case 0b11101110:
        case 0b11101111:
            return I_OUTPUT_TO_VARIABLE_PORT;

        // XLAT
        case 0b11010111:
            return I_TRANSLATE_BYTE_TO_AL;

        // LEA
        case 0b10001101:
            return I_LOAD_EA_TO_REGISTER;

        // LDS
        case 0b11000101:
            return I_LOAD_POINTER_TO_DS;

        // LES
        case 0b11000100:
            return I_LOAD_POINTER_TO_ES;

        // LAHF
        case 0b10011111:
            return I_LOAD_AH_WITH_FLAGS;

        // SAHF
        case 0b10011110:
            return I_STORE_AH_INTO_FLAGS;

        // PUSHF
        case 0b10011100:
            return I_PUSH_FLAGS;

        // POPF
        case 0b10011101:
            return I_POP_FLAGS;

        // MARK: COMMON 0b100000xx
        case 0b10000000:
        case 0b10000001:
        case 0b10000010:
        case 0b10000011: {
            uint8_t sub_opcode = byte2 & 0b00111000;
            switch(sub_opcode) {
                // ADD 2
                case 0b00000000:
                    return I_ADD_IMMEDIATE;
                // ADC 2
                case 0b00010000:
                    return I_ADC_IMMEDIATE;
                // SUB 2
                case 0b00101000:
                    return I_SUB_IMMEDIATE;
                // SBB 2
                case 0b00011000:
                    return I_SBB_IMMEDIATE;
                // CMP 2
                case 0b00111000:
                    return I_COMPARE_IMMEDIATE;
                // AND 2
                case 0b00100000:
                    return I_AND_IMMEDIATE;
                // OR 2
                case 0b00001000:
                    return I_OR_IMMEDIATE_TO_ACCUMULATOR;
                // XOR 2
                case 0b00110000:
                    return I_XOR_IMMEDIATE_TO_REGISTER_OR_MEMORY;
                default:
                    break;
            };
            break;
        }

        // MARK: COMMON 0b1111011x
        case 0b11110110:
        case 0b11110111: {
            uint8_t sub_opcode = byte2 & 0b00111000;
            switch(sub_opcode) {
                // NEG
                case 0b00011000:
                    return I_NEGATE_CHANGE_SIGN;

                // MUL
                case 0b000100000:
                    return I_MULTIPLY_UNSIGNED;

                // IMUL
                case 0b00101000:
                    return I_INTEGER_MULTIPLY_SIGNED;

                // DIV
                case 0b00110000:
                    return I_DIVIDE_UNSIGNED;

                // IDIV
                case 0b00111000:
                    return I_INTEGER_DIVIDE_SIGNED;

                // NOT
                case 0b00010000:
                    return I_NOT;

                // TEST 2
                case 0b00000000:
                    return I_TEST_IMMEDIATE_DATA_AND_REGISTER_OR_MEMORY;

                default:
                    break;
            };
            break;
        }

        // MARK: ADD
        // ADD 1 - 0b000000xx
        case 0b00000000:
        case 0b00000001:
        case 0b00000010:
        case 0b00000011:
            return I_ADD;
        // ADD 2 - I_ADD_IMMEDIATE - handled by "COMMON 0b100000xx"
        // ADD 3 - 0b0000010x
        case 0b00000100:
        case 0b00000101:
            return I_ADD_IMMEDIATE_TO_AX;

        // MARK: ADC
        // ADC 1 - 0b000100xx
        case 0b00010000:
        case 0b00010001:
        case 0b00010010:
        case 0b00010011:
            return I_ADC;
        // ADC 2 - I_ADC_IMMEDIATE - handled by "COMMON 0b100000xx"
        // ADC 3 - 0b0001010x
        case 0b00010100:
        case 0b00010101:
            return I_ADC_IMMEDIATE_TO_AX;

        // MARK: INC
        // INC 1 - I_INC - handled by "COMMON 0b11111110" and "COMMON 0b11111111"
        // INC 2 - 0b01000xxx
        case 0b01000000:
        case 0b01000001:
        case 0b01000010:
        case 0b01000011:
        case 0b01000100:
        case 0b01000101:
        case 0b01000110:
        case 0b01000111:
            return I_INC_REGISTER;

        // AAA
        case 0b00110111:
            return I_AAA_ASCII_ADJUST_FOR_ADD;

        // DAA
        case 0b00100111:
            return I_DAA_DECIMAL_ADJUST_FOR_ADD;

        // MARK: SUB
        // SUB 1 - 0b001010xx
        case 0b00101000:
        case 0b00101001:
        case 0b00101010:
        case 0b00101011:
            return I_SUB;
        // SUB 2 - I_SUB_IMMEDIATE - handled by common "COMMON 0b100000xx"
        // SUB 3 - 0b0010110x
        case 0b00101100:
        case 0b00101101:
            return I_SUB_IMMEDIATE_FROM_AX;

        // MARK: SBB
        // SBB 1 - 0b001010xx
        case 0b00011000:
        case 0b00011001:
        case 0b00011010:
        case 0b00011011:
            return I_SBB;
        // SBB 2 - I_SBB_IMMEDIATE - handled by common "COMMON 0b100000xx"
        // SBB 3 - 0b0010110x
        case 0b00011100:
        case 0b00011101:
            return I_SBB_IMMEDIATE_FROM_AX;

        // MARK: DEC
        // DEC 1 - I_DEC_REGISTER_OR_MEMORY - handled by "COMMON 0b11111110" and "COMMON 0b11111111"
        // DEC 2 - 0b01001xxx
        case 0b01001000:
        case 0b01001001:
        case 0b01001010:
        case 0b01001011:
        case 0b01001100:
        case 0b01001101:
        case 0b01001110:
        case 0b01001111:
            return I_DEC_REGISTER;

        // NEG - I_NEGATE_CHANGE_SIGN - handled by "COMMON 0b1111011x"

        // MARK: CMP
        // CMP 1 - 0b001110xx
        case 0b00111000:
        case 0b00111001:
        case 0b00111010:
        case 0b00111011:
            return I_COMPARE;
        // CMP 2 - I_COMPARE_IMMEDIATE - handled by "COMMON 0b100000xx"
        // CMP 3 - 0b0011110x
        case 0b00111100:
        case 0b00111101:
            return I_COMPARE_IMMEDIATE_WITH_AX;

        // AAS
        case 0b00111111:
            return I_ASCII_ADJUST_FOR_SUBTRACT;

        // DAS
        case 0b00101111:
            return I_DECIMAL_ADJUST_FOR_SUBTRACT;

        // MUL - I_MULTIPLY_UNSIGNED - handled by "COMMON 0b1111011x"
        // IMUL - I_INTEGER_MULTIPLY_SIGNED - handled by "COMMON 0b1111011x"
        // AAM
        case 0b11010100:
            return I_ASCII_ADJUST_FOR_MULTIPLY;

        // DIV - handled by "COMMON 0b1111011x"
        // IDIV - handled by "COMMON 0b1111011x"
        // AAD
        case 0b11010101:
            return I_ASCII_ADJUST_FOR_DIVIDE;

        // CBW
        case 0b10011000:
            return I_CONVERT_BYTE_TO_WORD;
        // CWD
        case 0b10011001:
            return I_CONVERT_WORD_TO_DOUBLE_WORD;

        // MARK: COMMON 0b110100xx
        case 0b11010000:
        case 0b11010001:
        case 0b11010010:
        case 0b11010011:{
            uint8_t sub_opcode = byte2 & 0b00111000;
            switch(sub_opcode) {
                // SHL
                case 0b00100000:
                    return I_SHIFT_LOGICAL_LEFT;
                // SHR
                case 0b00101000:
                    return I_SHIFT_LOGICAL_RIGHT;
                // SAR
                case 0b00111000:
                    return I_SHIFT_ARITHMETIC_RIGHT;
                // ROL
                case 0b00000000:
                    return I_ROTATE_LEFT;
                // ROR
                case 0b00001000:
                    return I_ROTATE_RIGHT;
                // RCL
                case 0b00010000:
                    return I_ROTATE_LEFT_CARRY;
                // RCR
                case 0b00011000:
                    return I_ROTATE_RIGHT_CARRY;
                default:
                    break;
            };
            break;
        }

        // MARK: AND
        // AND 1 - I_AND - 0b0010000xx
        case 0b00100000:
        case 0b00100001:
        case 0b00100010:
        case 0b00100011:
            return I_AND;
        // AND 2 - I_AND_IMMEDIATE - handled by common
        // AND 3 - I_AND_IMMEDIATE_TO_AX - 0b0010010x
        case 0b00100100:
        case 0b00100101:
            return I_AND_IMMEDIATE_TO_AX;

        // MARK: TEST
        // TODO: TEST1 opcodes overlap with ADC opcodes. Not sure how to differentiate.
        // TEST 1 - I_TEST_REGISTER_OR_MEMORY_AND_REGISTER - 0b000100xx
        // case 0b00010000:
        // case 0b00010001:
        // case 0b00010010:
        // case 0b00010011:
        //     return I_TEST_REGISTER_OR_MEMORY_AND_REGISTER;
        // TEST 2 - I_TEST_IMMEDIATE_DATA_AND_REGISTER_OR_MEMORY - handled by "COMMON 0b1111011x"
        // TEST 3 - I_TEST_IMMEDIATE_DATA_AND_ACCUMULATOR - 0b1010100x
        case 0b10101000:
        case 0b10101001:
            return I_TEST_IMMEDIATE_DATA_AND_ACCUMULATOR;

        // MARK: OR
        // OR 1 - I_OR_REGISTER_OR_MEMORY_AND_REGISTER_TO_EITHER - 0b000010xx
        case 0b00001000:
        case 0b00001001:
        case 0b00001010:
        case 0b00001011:
            return I_OR_REGISTER_OR_MEMORY_AND_REGISTER_TO_EITHER;
        // OR 2 - I_OR_IMMEDIATE_TO_REGISTER_OR_MEMORY - handled by "COMMON 0b100000xx"
        // OR 3 - I_OR_IMMEDIATE_TO_ACCUMULATOR - 0b0000110x
        case 0b00001100:
        case 0b00001101:
            return I_OR_IMMEDIATE_TO_ACCUMULATOR;

        // MARK: XOR
        // XOR 1 - I_XOR_REGISTER_OR_MEMORY_AND_REGISTER_TO_EITHER - 0b001100xx
        case 0b00110000:
        case 0b00110001:
        case 0b00110010:
        case 0b00110011:
            return I_XOR_REGISTER_OR_MEMORY_AND_REGISTER_TO_EITHER;
        // XOR 2 - I_XOR_IMMEDIATE_TO_REGISTER_OR_MEMORY - handled by "COMMON 0b100000xx"
        // NOTE: typo in data sheet. I'm assuming the first byte field should
        // be "1000000w", and second byte field should be: "mod ??? r/m". This matches
        // "OR 2"s pattern. Found the sub-opcode "110" and verified above with NASM.
        // nasm "mov bl, 12" -> 0b10000000 0b11110011 0b00001100
        // XOR 3 - I_XOR_IMMEDIATE_TO_ACCUMULATOR - 0b0011010x
        case 0b00110100:
        case 0b00110101:
            return I_XOR_IMMEDIATE_TO_ACCUMULATOR;

        //
        // MARK: STRING MANIPULATION
        //

        // REP - 0b1111001x
        case 0b11110010:
        case 0b11110011:
            return I_REPEAT;

        // MOVS - 0b1010010x
        case 0b10100100:
        case 0b10100101:
            return I_STRING_MOVE;

        // CMPS - 0b1010011x
        case 0b10100110:
        case 0b10100111:
            return I_STRING_COMPARE;

        // SCAS - 0b1010111x
        case 0b10101110:
        case 0b10101111:
            return I_STRING_SCAN;

        // LODS - 0b1010110x
        case 0b10101100:
        case 0b10101101:
            return I_STRING_LOAD;

        // STDS - 0b1010101x
        case 0b10101010:
        case 0b10101011:
            return I_STRING_STORE;

        //
        // MARK: CONTROL TRANSFER
        //

        // MARK: CALL
        // CALL 1
        case 0b11101000:
            return I_CALL_DIRECT_WITHIN_SEGMENT;
        // CALL 2 - handled in "COMMON 0b11111111"
        // CALL 3
        case 0b10011010:
            return I_CALL_DIRECT_INTERSEGMENT;
        // CALL 4 - handled in "COMMON 0b11111111"

        // MARK: JMP
        // JMP 1
        case 0b11101001:
            return I_JUMP_DIRECT_WITHIN_SEGMENT;
        // JMP 2
        case 0b11101011:
            return I_JUMP_DIRECT_WITHIN_SEGMENT_SHORT;
        // JMP 3 - handled in "COMMON 0b11111111"
        // JMP 4
        case 0b11101010:
            return I_JUMP_DIRECT_INTERSEGMENT;
        // JMP 5 - handled in "COMMON 0b11111111"

        // MARK: RET
        // RET 1
        case 0b11000011:
            return I_RETURN_WITHIN_SEGMENT;
        // RET 2
        case 0b011000010:
            return I_RETURN_WITHIN_SEGMENT_ADD_IMMEDIATE_TO_SP;
        // RET 3
        case 0b11001011:
            return I_RETURN_INTERSEGMENT;
        // RET 4
        case 0b11001010:
            return I_RETURN_INTERSEGMENT_ADD_IMMEDIATE_TO_SP;

        //
        // MARK: CONDITIONAL JUMPS
        //

        // JE/JZ
        case 0b01110100:
            return I_JUMP_ON_EQUAL;

        // JL/JNGE
        case 0b01111100:
            return I_JUMP_ON_LESS;

        // JLE/JNG
        case 0b01111110:
            return I_JUMP_ON_LESS_OR_EQUAL;

        // JB/JNAE
        case 0b01110010:
            return I_JUMP_ON_BELOW;

        // JBE/JNA
        case 0b01110110:
            return I_JUMP_ON_BELOW_OR_EQUAL;

        // JP/JPE
        case 0b01111010:
            return I_JUMP_ON_PARITY;

        // JO
        case 0b01110000:
            return I_JUMP_ON_OVERLFLOW;

        // JS
        case 0b01111000:
            return I_JUMP_ON_SIGN;

        // JNE/JNZ
        case 0b01110101:
            return I_JUMP_ON_NOT_EQUAL;

        // JNL/JGE
        case 0b01111101:
            return I_JUMP_ON_GREATER_OR_EQUAL;

        // JNLE/JG
        case 0b01111111:
            return I_JUMP_ON_GREATER;

        // JNB/JAE
        case 0b01110011:
            return I_JUMP_ON_ABOVE_OR_EQUAL;

        // JNBE/JA
        case 0b01110111:
            return I_JUMP_ON_ABOVE;

        // JNP/JPO
        case 0b01111011:
            return I_JUMP_ON_NOT_PARITY;

        // JNO
        case 0b01110001:
            return I_JUMP_ON_NOT_OVERFLOW;

        // JNS
        case 0b01111001:
            return I_JUMP_ON_NOT_SIGN;

        // MARK: LOOPS
        // LOOP
        case 0b11100010:
            return I_LOOP;
        // LOOPZ/LOOPE
        case 0b11100001:
            return I_LOOP_WHILE_EQUAL;
        // LOOPNZ/LOOPNE
        case 0b11100000:
            return I_LOOP_WHILE_NOT_EQUAL;
        // JCXZ
        case 0b11100011:
            return I_JUMP_ON_CX_ZERO;

        //
        // MARK: INTERRUPTS
        //

        // MARK: INT
        // INT 1
        case 0b11001101:
            return I_INTERRUPT_TYPE_SPECIFIED;
        // INT 2
        case 0b11001100:
            return I_INTERRUPT_TYPE_3;

        // INTO
        case 0b11001110:
            return I_INTERRUPT_ON_OVERFLOW;
        // IRET
        case 0b11001111:
            return I_INTERRUPT_RETURN;

        //
        // MARK: PROCESSOR CONTROL
        //

        // CLC
        case 0b11111000:
            return I_CLEAR_CARRY;

        // CMC
        case 0b11110101:
            return I_COMPLEMENT_CARRY;

        // STC
        case 0b11111001:
            return I_SET_CARRY;

        // CLD
        case 0b11111100:
            return I_CLEAR_DIRECTION;

        // STD
        case 0b11111101:
            return I_SET_DIRECTION;

        // CLI
        case 0b11111010:
            return I_CLEAR_INTERRUPT;

        // STI
        case 0b11111011:
            return I_SET_INTERRUPT;

        // HTL
        case 0b11110100:
            return I_HALT;

        // WAIT
        case 0b10011011:
            return I_WAIT;

        // ESC - 0b11011xxx
        case 0b11011000:
        case 0b11011001:
        case 0b11011010:
        case 0b11011011:
        case 0b11011100:
        case 0b11011101:
        case 0b11011110:
        case 0b11011111:
            return I_ESCAPE;

        // LOCK
        case 0b11110000:
            return I_LOCK;

        // SEGMENT - 0b001xx110
        case 0b00100110:
        case 0b00101110:
        case 0b00110110:
        case 0b00111110:
            return I_SEGMENT;
        default:
            break;
    }
    return I_INVALID;
}
