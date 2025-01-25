


#include <stdint.h>

#include "shared/include/instructions.h"

#include "libraries/decode8086/include/decode_tag.h"

/**
 * Reads the opcode of an instruction and determines the associated instruction tag.
 * @param byte1 Byte to read the opcode from.
 * @param byte2 Byte to read the opcode from.
 * @return Tag if the opcode matched or I_INVALID.
 */
instruction_tag_t dcd_decode_tag(uint8_t byte1, uint8_t byte2) {
    switch(byte1) {
        // MARK: MOV
        // MOV 1. I_MOVE_IMMEDIATE_TO_REGISTER - 0b100010xx
        case 0b10001000:
        case 0b10001001:
        case 0b10001010:
        case 0b10001011:
            return I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY;
        // MOV 2. I_MOVE_IMMEDIATE_TO_REGISTER - 0b100010xx
        case 0b11000110:
        case 0b11000111:
            return I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY;
        // MOV 3. I_MOVE_IMMEDIATE_TO_REGISTER - 0b1011xxxx:
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
        // MOV 4. I_MOVE_MEMORY_TO_ACCUMULATOR - 0b1010000x
        case 0b10100000:
        case 0b10100001:
            return I_MOVE_MEMORY_TO_ACCUMULATOR;
        // MOV 5. I_MOVE_ACCUMULATOR_TO_MEMORY - 0b1010001x
        case 0b10100010:
        case 0b10100011:
            return I_MOVE_ACCUMULATOR_TO_MEMORY;
        // MOV 6. I_MOVE_REGISTER_OR_MEMORY_TO_SEGMENT_REGISTER - 0b10001110
        case 0b10001110:
            return I_MOVE_REGISTER_OR_MEMORY_TO_SEGMENT_REGISTER;
        // MOV 7. I_MOVE_SEGMENT_REGISTER_TO_REGISTER_OR_MEMORY - 0b10001100
        case 0b10001100:
            return I_MOVE_SEGMENT_REGISTER_TO_REGISTER_OR_MEMORY;

        // MARK: COMMON UNARY
        case 0b11111111: {
            uint8_t sub_opcode = byte2 & 0b00111000;
            switch(sub_opcode) {
                case 0b00110000:
                    return I_PUSH_REGISTER_OR_MEMORY;
                case 0b00000000:
                    return I_INC_REGISTER_OR_MEMORY;
                case 0b00001000:
                    return I_DEC_REGISTER_OR_MEMORY;
                default:
                    break;
            };
            break;
        }

        // MARK: COMMON UNARY 2
        // TODO: consider combining with common unary 1? push can't have a 0, but might be fine.
        case 0b11111110: {
            uint8_t sub_opcode = byte2 & 0b00111000;
            switch(sub_opcode) {
                case 0b00000000:
                    return I_INC_REGISTER_OR_MEMORY;
                case 0b00001000:
                    return I_DEC_REGISTER_OR_MEMORY;
                default:
                    break;
            };
            break;
        }

        // MARK: COMMON UNARY 3
        case 0b10001111: {
            uint8_t sub_opcode = byte2 & 0b00111000;
            switch(sub_opcode) {
                case 0b00000000:
                    return I_POP_REGISTER_OR_MEMORY;
                default:
                    break;
            };
            break;
        }

        // MARK: PUSH
        // PUSH 1. I_PUSH_REGISTER_OR_MEMORY - handled by common unary operation
        // PUSH 2. I_PUSH_REGISTER - 0b01010xxx
        case 0b01010000:
        case 0b01010001:
        case 0b01010010:
        case 0b01010011:
        case 0b01010100:
        case 0b01010101:
        case 0b01010110:
        case 0b01010111:
            return I_PUSH_REGISTER_OR_MEMORY;
        // PUSH 3. I_PUSH_SEGMENT_REGISTER - 0b000xxx110
        case 0b00000110:
        case 0b00001110:
        case 0b00010110:
        case 0b00011110:
        case 0b00100110:
        case 0b00101110:
        case 0b00110110:
        case 0b00111110:
            return I_PUSH_SEGMENT_REGISTER;

        // MARK: POP
        // POP 1. I_POP_REGISTER_OR_MEMORY - handled by common unary operation
        // POP 2. I_POP_REGISTER - 0b01011xxx
        case 0b01011000:
        case 0b01011001:
        case 0b01011010:
        case 0b01011011:
        case 0b01011100:
        case 0b01011101:
        case 0b01011110:
        case 0b01011111:
            return I_POP_REGISTER;
        // POP 3. I_POP_SEGMENT_REGISTER - 0b000xxx111
        case 0b000000111:
        case 0b000001111:
        case 0b000010111:
        case 0b000011111:
        case 0b000100111:
        case 0b000101111:
        case 0b000110111:
        case 0b000111111:
            return I_POP_SEGMENT_REGISTER;

        // MARK: XCHG
        // XCHG 1. I_EXCHANGE_REGISTER_OR_MEMORY_WITH_REGISTER - 0b1000011x
        case 0b10000110:
        case 0b10000111:
            return I_EXCHANGE_REGISTER_OR_MEMORY_WITH_REGISTER;
        // XCHG 2. I_EXCHANGE_REGISTER_WITH_ACCUMULATOR - 0b10010xxx
        case 0b10010000:
        case 0b10010001:
        case 0b10010010:
        case 0b10010011:
        case 0b10010100:
        case 0b10010101:
        case 0b10010110:
        case 0b10010111:
            return I_EXCHANGE_REGISTER_WITH_ACCUMULATOR;

        // MARK: IN
        // IN 1. I_INPUT_FROM_FIXED_PORT - 0b1110010x
        case 0b11100100:
        case 0b11100101:
            return I_INPUT_FROM_FIXED_PORT;
        // IN 2. I_INPUT_FROM_VARIABLE_PORT - 0b1110110x
        case 0b11101100:
        case 0b11101101:
            return I_INPUT_FROM_VARIABLE_PORT;

        // MARK: OUT
        // OUT 1. I_OUTPUT_TO_FIXED_PORT - 0b1110011x
        case 0b11100110:
        case 0b11100111:
            return I_OUTPUT_TO_FIXED_PORT;
        // OUT 2. I_OUTPUT_TO_VARIABLE_PORT - 0b1110110x
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

        // MARK: COMMON BINARY
        // 0b100000xx
        case 0b10000000:
        case 0b10000001:
        case 0b10000010:
        case 0b10000011: {
            uint8_t sub_opcode = byte2 & 0b00111000;
            switch(sub_opcode) {
                case 0b00000000:
                    return I_ADD_IMMEDIATE_TO_REGISTER_OR_MEMORY;
                case 0b00010000:
                    return I_ADC_IMMEDIATE_TO_REGISTER_OR_MEMORY;
                case 0b00101000:
                    return I_SUB_IMMEDIATE_FROM_REGISTER_OR_MEMORY;
                case 0b00011000:
                    return I_SBB_IMMEDIATE_FROM_REGISTER_OR_MEMORY;
                case 0b00111000:
                    return I_COMPARE_IMMEDIATE_WITH_REGISTER_OR_MEMORY;
                default:
                    break;
            };
            break;
        }

        // MARK: ADD
        // ADD 1. I_ADD_REGISTER_OR_MEMORY_WITH_REGISTER_TO_EITHER - 0b000000xx
        case 0b00000000:
        case 0b00000001:
        case 0b00000010:
        case 0b00000011:
            return I_ADD_REGISTER_OR_MEMORY_WITH_REGISTER_TO_EITHER;
        // ADD 2. I_ADD_IMMEDIATE_TO_REGISTER_OR_MEMORY - handled by common immediate operation
        // ADD 3. I_ADD_IMMEDIATE_TO_ACCUMULATOR - 0b0000010x
        case 0b00000100:
        case 0b00000101:
            return I_ADD_IMMEDIATE_TO_ACCUMULATOR;

        // MARK: ADC
        // ADC 1. I_ADC_REGISTER_OR_MEMORY_WITH_REGISTER_TO_EITHER - 0b000100xx
        case 0b00010000:
        case 0b00010001:
        case 0b00010010:
        case 0b00010011:
            return I_ADC_REGISTER_OR_MEMORY_WITH_REGISTER_TO_EITHER;
        // ADC 2. I_ADC_IMMEDIATE_TO_REGISTER_OR_MEMORY - handled by common
        // ADC 3. I_ADC_IMMEDIATE_TO_ACCUMULATOR - 0b0001010x
        case 0b00010100:
        case 0b00010101:
            return I_ADC_IMMEDIATE_TO_ACCUMULATOR;

        // MARK: INC
        // INC 1. I_INC_REGISTER_OR_MEMORY - handled by common unary
        // INC 2. I_INC_REGISTER - 0b01000xxx
        case 0b01000000:
        case 0b01000001:
        case 0b01000010:
        case 0b01000011:
        case 0b01000100:
        case 0b01000101:
        case 0b01000110:
        case 0b01000111:
            return I_INC_REGISTER;
        // TOOD: overlap between AAA/DAA that needs to be figured out
        // // AAA - I_AAA_ASCII_ADJUST_FOR_ADD - 0b00110111
        // case 0b00110111:
        //     return I_AAA_ASCII_ADJUST_FOR_ADD;
        // // DAA - I_DAA_DECIMAL_ADJUST_FOR_ADD - 0b00100111
        // case 0b00100111:
        //     return I_DAA_DECIMAL_ADJUST_FOR_ADD;

        // MARK: SUB
        // SUB 1. I_SUB_REGISTER_OR_MEMORY_AND_REGISTER_TO_EITHER - 0b001010xx
        case 0b00101000:
        case 0b00101001:
        case 0b00101010:
        case 0b00101011:
            return I_SUB_REGISTER_OR_MEMORY_AND_REGISTER_TO_EITHER;
        // SUB 2. I_SUB_IMMEDIATE_FROM_REGISTER_OR_MEMORY - handled by common
        // SUB 3. I_SUB_IMMEDIATE_FROM_ACCUMULATOR - 0b0010110x
        case 0b00101100:
        case 0b00101101:
            return I_SUB_IMMEDIATE_FROM_ACCUMULATOR;

        // MARK: SBB
        // SBB 1. I_SBB_REGISTER_OR_MEMORY_AND_REGISTER_TO_EITHER - 0b001010xx
        case 0b00011000:
        case 0b00011001:
        case 0b00011010:
        case 0b00011011:
            return I_SBB_REGISTER_OR_MEMORY_AND_REGISTER_TO_EITHER;
        // SBB 2. I_SBB_IMMEDIATE_FROM_REGISTER_OR_MEMORY - handled by common
        // SBB 3. I_SBB_IMMEDIATE_FROM_ACCUMULATOR - 0b0010110x
        case 0b00011100:
        case 0b00011101:
            return I_SBB_IMMEDIATE_FROM_ACCUMULATOR;

        // MARK: DEC
        // DEC 1. I_DEC_REGISTER_OR_MEMORY - handled by common
        // DEC 2. I_DEC_REGISTER - 0b01001xxx
        case 0b01001000:
        case 0b01001001:
        case 0b01001010:
        case 0b01001011:
        case 0b01001100:
        case 0b01001101:
        case 0b01001110:
        case 0b01001111:
            return I_DEC_REGISTER;

        // NEG - 0b1111011x
        case 0b11110110:
        case 0b11110111:
            return I_NEGATE_CHANGE_SIGN;
        // MARK: CMP
        // CMP 1. I_COMPARE_REGISTER_OR_MEMORY_AND_REGISTER - 0b001110xx
        case 0b00111000:
        case 0b00111001:
        case 0b00111010:
        case 0b00111011:
            return I_COMPARE_REGISTER_OR_MEMORY_AND_REGISTER;
        // CMP 2. I_COMPARE_IMMEDIATE_WITH_REGISTER_OR_MEMORY - handled by common
        // CMP 3. I_COMPARE_IMMEDIATE_WITH_ACCUMULATOR - 0b0011110x
        case 0b00111100:
        case 0b00111101:
            return I_COMPARE_IMMEDIATE_WITH_ACCUMULATOR;
        
        // AAS
        // DAS
        // MUL
        // IMUL
        // AAM
        // TODO: the rest of the owl
        default:
            break;
    }
    return I_INVALID;
}