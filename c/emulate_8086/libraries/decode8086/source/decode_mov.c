/**
 * Responsible for decoding the "mov" assembly instruction for the 8086. This instruction
 * can be mapped into 7 different opcode encodings. The order of these encodings will
 * follow the datasheet table 4-12.
 * 
 * 1. I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY
 * 2. I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY
 * 3. I_MOVE_IMMEDIATE_TO_REGISTER
 * 4. I_MOVE_MEMORY_TO_ACCUMULATOR
 * 5. I_MOVE_ACCUMULATOR_TO_MEMORY
 * 6. I_MOVE_REGISTER_OR_MEMORY_TO_SEGMENT_REGISTER
 * 7. I_MOVE_SEGMENT_REGISTER_TO_REGISTER_OR_MEMORY
 * 
 */

#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"

#include "libraries/decode8086/include/decode8086.h"
#include "libraries/decode8086/include/decode_utils.h"
#include "libraries/decode8086/include/decode_mov.h"



// MARK: 1. I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY

decode_result_t decode__move_register_or_memory_to_or_from_register_or_memory(
    decoder_t* decoder,
    uint8_t byte1,
    move_register_or_memory_to_or_from_register_or_memory_t* move)
{
    move->fields1 = byte1;
    direction_t direction = (move->fields1 & 0b00000010) >> 1;
    wide_t wide = move->fields1 & 0b00000001;
    decode_result_t read_byte2_result = dcd_read_byte(decoder, (uint8_t*) &move->fields2);
    if (read_byte2_result != DR_SUCCESS) {
        return read_byte2_result;
    }

    mod_t mod = (move->fields2 & 0b11000000) >> 6;
    reg_t reg = (move->fields2 & 0b00111000) >> 3;
    uint8_t rm = move->fields2 & 0b00000111;
    if (mod == MOD_MEMORY) {
        if (rm == 0b00000110) {
            decode_result_t read_displace_result = dcd_read_word(decoder, &move->displacement);
            if (read_displace_result != DR_SUCCESS) { 
                return read_displace_result;
            }
        }
    } else if (mod == MOD_MEMORY_8BIT_DISPLACEMENT) {
        decode_result_t read_displace_result = dcd_read_byte(decoder, (uint8_t*) &move->displacement);
        if (read_displace_result != DR_SUCCESS) { 
            return read_displace_result;
        }
    } else if (mod == MOD_MEMORY_16BIT_DISPLACEMENT) {
        decode_result_t read_displace_result = dcd_read_word(decoder, &move->displacement);
        if (read_displace_result != DR_SUCCESS) { 
            return read_displace_result; 
        }
    } else { // MOD_REGISTER
        // Don't have extra bytes for register to register movs. Nothing to do.
    }

    return SUCCESS;
}

char* mod_memory_effective_address_lookup(uint8_t rm) {
    switch(rm) {
        case RM_BX_SI: // 0
            return "[bp + si]";
        case RM_BX_DI: // 1
            return "[bx + di]";
        case RM_BP_SI:
            return "[bp + si]";
        case RM_BP_DI:
            return "[bp + di]";
        case RM_SI:
            return "[si]";
        case RM_DI:
            return "[di]";
        case RM_BP:
            return "INVALID";
        case RM_BX:
            return "[bx]";
        default:
            return "INVALID";
    };
}

void write_uint8(char* buffer, int* index, size_t buffer_size, uint8_t num) {
    // TODO: write something optimized. just use sprintf for now
    snprintf(buffer + *index, buffer_size - *index, "%d", num);
}

void write_uint16(char* buffer, int* index, size_t buffer_size, uint16_t num) {
    // TODO: write something optimized. just use sprintf for now
    snprintf(buffer + *index, buffer_size - *index, "%d", num);
}

void build_effective_address(char* buffer, size_t buffer_size, 
                             mod_t mod, uint8_t rm, uint16_t displacement) {
    switch(mod) {
        case MOD_MEMORY:
            switch(rm) {
                case RM_BX_SI:
                    memcpy(buffer, "[bx + si]", sizeof("[bx + si]"));
                    break;
                case RM_BX_DI:
                    memcpy(buffer, "[bx + di]", sizeof("[bx + di]"));
                    break;
                case RM_BP_SI:
                    memcpy(buffer, "[bp + si]", sizeof("[bp + si]"));
                    break;
                case RM_BP_DI:
                    memcpy(buffer, "[bp + di]", sizeof("[bp + di]"));
                    break;
                case RM_SI:
                    memcpy(buffer, "[si]", sizeof("[si]"));
                    break;
                case RM_DI:
                    memcpy(buffer, "[di]", sizeof("[di]"));
                    break;
                case RM_BP:
                    memcpy(buffer, "INVALID", sizeof("INVALID"));
                    break;
                case RM_BX:
                    memcpy(buffer, "[bx]", sizeof("[bx]"));
                    break;
                default:
                    memcpy(buffer, "INVALID", sizeof("INVALID"));
                    break;
            };
            break;
        case MOD_MEMORY_8BIT_DISPLACEMENT:
        // TODO: idea for performance, instead of falling through here, could build the string
        // with a "int to string" function tuned for each 8 bits and a different one tuned and
        // called for 16 bits.
        case MOD_MEMORY_16BIT_DISPLACEMENT:
            // TODO: if displacement is 0, for example "[bp + 0]", maybe exclude the + 0?
            switch(rm) {
                case RM_BX_SI:
                    snprintf(buffer, buffer_size, "[bx + si + %d]", displacement);
                    break;
                case RM_BX_DI:
                    snprintf(buffer, buffer_size, "[bx + di + %d]", displacement);
                    break;
                case RM_BP_SI:
                    snprintf(buffer, buffer_size, "[bp + si + %d]", displacement);
                    break;
                case RM_BP_DI:
                    snprintf(buffer, buffer_size, "[bp + di + %d]", displacement);
                    break;
                case RM_SI:
                    snprintf(buffer, buffer_size, "[si + %d]", displacement);
                    break;
                case RM_DI:
                    snprintf(buffer, buffer_size, "[di + %d]", displacement);
                    break;
                case RM_BP:
                    // BP with memory mode/no displacement is hijacked for direct immediates, 
                    // so just [bp] gets converted to [bp + 0] displacement. 
                    // ex: "mov dx, [bp]" is actually: "mov dx, [bp + 0]"
                    if (displacement == 0) {
                        snprintf(buffer, buffer_size, "[bp]");
                    } else {
                        snprintf(buffer, buffer_size, "[bp + %d]", displacement);
                    }
                    break;
                case RM_BX:
                    snprintf(buffer, buffer_size, "[bx + %d]", displacement);
                    break;
                default:
                    memcpy(buffer, "INVALID", sizeof("INVALID"));
                    break;
            }
            break;
        case MOD_REGISTER:
        default:
            memcpy(buffer, "INVALID", sizeof("INVALID"));
            break;
    };
}

void write__move_register_or_memory_to_or_from_register_or_memory(
    move_register_or_memory_to_or_from_register_or_memory_t* move, 
    char* buffer,
    int* index,
    int buffer_size)
{
    wide_t wide = move->fields1 & 0b00000001;
    direction_t direction = (move->fields1 & 0b00000010) >> 1;
    mod_t mod = (move->fields2 & 0b11000000) >> 6;
    uint8_t reg = (move->fields2 & 0b00111000) >> 3;
    uint8_t rm = move->fields2 & 0b00000111;

    if (mod == MOD_REGISTER) {
        uint8_t* left = &rm;
        uint8_t* right = (uint8_t*)&reg;

        if (direction == DIR_FROM_REGISTER) {
            left = (uint8_t*)&reg;
            right = &rm;
        }

        char* left_string = regb_strings[*left];
        char* right_string = regb_strings[*right];

        if (wide == WIDE_WORD) {
            left_string = regw_strings[*left];
            right_string = regw_strings[*right];
        }

        int written = snprintf(buffer + *index, buffer_size - *index, "mov %s, %s",
                                left_string,
                                right_string);
        if (written < 0) {
            // TODO: propogate error
        }
        *index += written;
    } else {
        // Special hi-jacked case for immediate value address "direct loading"
        if (mod == MOD_MEMORY && rm == RM_BP) {
            if (direction == DIR_TO_REGISTER) {
                char* reg_string = regb_strings[reg];
                if (wide == WIDE_WORD) {
                    reg_string = regw_strings[reg];
                }
                int written = snprintf(buffer + *index,  buffer_size - *index, "mov [%d], %s", 
                                            move->displacement,
                                            reg_string);
                if (written < 0) {
                    // TODO: propogate error
                }
                *index += written;
            } else {
                char* reg_string = regb_strings[reg];
                if (wide == WIDE_WORD) {
                    reg_string = regw_strings[reg];
                }
                int written = snprintf(buffer + *index, buffer_size - *index, "mov %s, [%d]",
                                        reg_string,
                                        move->displacement);
                if (written < 0) {
                    // TODO: propogate error
                }
                *index += written;
            }
            
        } else {
            if (direction == DIR_TO_REGISTER) {
                char* reg_string = regb_strings[reg];
                if (wide == WIDE_WORD) {
                    reg_string = regw_strings[reg];
                }
                char effective_address_string[32] = { 0 };
                build_effective_address(effective_address_string, sizeof(effective_address_string),
                                        mod, rm, move->displacement);
                int written = snprintf(buffer + *index,  buffer_size - *index, "mov %s, %s",
                                        effective_address_string,
                                        reg_string);
                if (written < 0) {
                    // TODO: propogate error
                }
                *index += written;
            } else {
                char* reg_string = regb_strings[reg];
                if (wide == WIDE_WORD) {
                    reg_string = regw_strings[reg];
                }
                char effective_address_string[32] = { 0 };
                build_effective_address(effective_address_string, sizeof(effective_address_string),
                                        mod, rm, move->displacement);
                int written = snprintf(buffer + *index, buffer_size - *index, "mov %s, %s",
                                        reg_string,
                                        effective_address_string);
                if (written < 0) {
                    // TODO: propogate error
                }
                *index += written;
            }
            //snprintf(buffer, buffer_size, "NOT IMPLEMENTED!");
        }
    }
}

// MARK: 2. I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY

decode_result_t decode__move_immediate_to_register_or_memory(
    decoder_t* decoder,
    uint8_t byte1,
    move_immediate_to_register_or_memory_t* move) 
{
    move->fields1 = byte1;
    wide_t wide = move->fields1 & 0b00000001;

    if (dcd_read_byte(decoder, (uint8_t*) &move->fields2) == RI_FAILURE) { 
        return DR_UNKNOWN_OPCODE;
    }
    mod_t mod = move->fields2 & 0b00000001;
    uint8_t rm = move->fields2 & 0b00000111;

    printf("mod: %d, rm: %d\n", mod, rm);
    if (mod == MOD_MEMORY) {
        if (rm == 0b00000110) {
            decode_result_t read_displace_result = dcd_read_word(decoder, &move->displacement);
            if (read_displace_result != DR_SUCCESS) { 
                return read_displace_result;
            }
        }
    } else if (mod == MOD_MEMORY_8BIT_DISPLACEMENT) {
        decode_result_t read_displace_result = dcd_read_byte(decoder, (uint8_t*) &move->displacement);
        if (read_displace_result != DR_SUCCESS) { 
            return read_displace_result;
        }
    } else if (mod == MOD_MEMORY_16BIT_DISPLACEMENT) {
        printf("reading displacement\n");
        decode_result_t read_displace_result = dcd_read_word(decoder, &move->displacement);
        if (read_displace_result != DR_SUCCESS) { 
            return read_displace_result; 
        }
    } else { // MOD_REGISTER
        // Don't have extra bytes for register to register movs. Nothing to do.
    }

    if (wide == WIDE_BYTE) {
        decode_result_t read_data_result = dcd_read_byte(decoder, (uint8_t*) &move->immediate);
    } else {
        printf("reading immediate\n");
        decode_result_t read_data_result = dcd_read_word(decoder, &move->immediate);
    }
    
    return SUCCESS;
}

void write__move_immediate_to_register_or_memory(
    move_immediate_to_register_or_memory_t* move, 
    char* buffer,
    int* index,
    int buffer_size)
{
    wide_t wide = move->fields1 & 0b00000001;
    mod_t mod = (move->fields2 & 0b11000000) >> 6;
    //uint8_t reg = (move->fields2 & 0b00111000) >> 3;
    uint8_t rm = move->fields2 & 0b00000111;

    char effective_address_string[32] = { 0 };
    build_effective_address(effective_address_string, sizeof(effective_address_string),
                            mod, rm, move->displacement);
    int written = snprintf(buffer + *index,  buffer_size - *index, "mov %s, %d", 
                            effective_address_string,
                            move->immediate);
    if (written < 0) {
        // TODO: propogate error
    }
    *index += written;



    //snprintf(buffer, buffer_size, "TODO: I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY!");
    
    // char* left_string = "TODO";
    // char* right_string = "TODO";
    // snprintf(buffer, buffer_size, "%s %s, %s", 
    //             instruction_metadata[I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY].mnemonic,
    //             left_string,
    //             right_string);
}

// MARK: 3. I_MOVE_IMMEDIATE_TO_REGISTER
decode_result_t decode__move_immediate_to_register(
    decoder_t* decoder,
    uint8_t byte1,
    move_immediate_to_register_t* move)
{
    move->fields1 = byte1;
    uint8_t wide = (move->fields1 & 0b00001000) >> 3;
    uint8_t reg = move->fields1 & 0b00000111;

    if (wide == WIDE_BYTE) {
        decode_result_t read_data_result = dcd_read_byte(decoder, (uint8_t*)&move->immediate);
        printf("immed to reg data: %d\n", move->immediate);
        if (read_data_result != DR_SUCCESS) {
            return FAILURE;
        }
    } else { // WIDE_WORD
        decode_result_t read_data_result = dcd_read_word(decoder, &move->immediate);
        printf("immed to reg (wide) data: %d\n", move->immediate);
        if (read_data_result != DR_SUCCESS) {
            return FAILURE;
        }
    }
    return DR_SUCCESS;
}

void write__move_immediate_to_register(
    move_immediate_to_register_t* move, 
    char* buffer,
    int* index,
    int buffer_size)
{
    uint8_t wide = (move->fields1 & 0b00001000) >> 3;
    uint8_t reg = move->fields1 & 0b00000111;

    char* reg_string = regb_strings[reg];
    if (wide == WIDE_WORD) {
        reg_string = regw_strings[reg];
    }

    int written = snprintf(buffer + *index, buffer_size - *index, "mov %s, %d",
                            reg_string,
                            move->immediate);
    if (written < 0) {
        // TODO: propogate error
    }
    *index += written;
}

// MARK: 4. I_MOVE_MEMORY_TO_ACCUMULATOR
decode_result_t decode__move_memory_to_accumulator(
    decoder_t* decoder,
    uint8_t byte1,
    move_memory_to_accumulator_t* move
) {
    move->fields1 = byte1;
    wide_t wide = move->fields1 & 0b00000001;
    decode_result_t read_data_result = dcd_read_word(decoder, &move->address);
    printf("address: %d\n", move->address);
    if (read_data_result != DR_SUCCESS) {
        return DR_FAILURE;
    }
    return DR_SUCCESS;
}

void write__move_memory_to_accumulator(
    move_memory_to_accumulator_t* move, 
    char* buffer,
    int* index,
    int buffer_size
) {
    wide_t wide = move->fields1 & 0b00000001;
    char* reg_string = "al";
    if (wide == WIDE_WORD) {
        reg_string = "ax";
    }

    int written = snprintf(buffer + *index, buffer_size - *index, "mov %s, [%d]",
                            reg_string,
                            move->address);
    if (written < 0) {
        // TODO: propogate error
    }
    *index += written;
}

// MARK: 5. I_MOVE_ACCUMULATOR_TO_MEMORY
decode_result_t decode__move_accumulator_to_memory(
    decoder_t* decoder,
    uint8_t byte1,
    move_accumulator_to_memory_t* move
) {
    move->fields1 = byte1;
    wide_t wide = move->fields1 & 0b00000001;
    decode_result_t read_data_result = dcd_read_word(decoder, &move->address);
    if (read_data_result != DR_SUCCESS) {
        return DR_FAILURE;
    }
    return DR_SUCCESS;
}

void write__move_accumulator_to_memory(
    move_accumulator_to_memory_t* move, 
    char* buffer,
    int* index,
    int buffer_size
) {
    wide_t wide = move->fields1 & 0b00000001;
    char* reg_string = "al";
    if (wide == WIDE_WORD) {
        reg_string = "ax";
    }

    int written = snprintf(buffer + *index, buffer_size - *index, "mov [%d], %s",
                            move->address,
                            reg_string);
    if (written < 0) {
        // TODO: propogate error
    }
    *index += written;
}