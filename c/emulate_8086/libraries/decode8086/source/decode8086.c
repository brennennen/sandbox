

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"


#include "libraries/decode8086/include/decode8086.h"
//#include "decode8086.h"


void dcd_init(decoder_t* decoder, instruction_t* instructions, size_t instructions_size) {
    decoder->instructions = instructions;
    decoder->instructions_capacity = instructions_size;
    decoder->instructions_count = 0;
}

result_iter_t dcd_read_byte(decoder_t* decoder, uint8_t* out_byte) {
    if (decoder->buffer_index >= decoder->buffer_size) {
        return RI_FAILURE;
    }
    *out_byte = decoder->buffer[decoder->buffer_index];
    decoder->buffer_index += 1;
    if (decoder->buffer_index >= decoder->buffer_size) {
        return RI_DONE;
    }
    return RI_CONTINUE;
}

result_iter_t dcd_read_word(decoder_t* decoder, uint16_t* out_word) {
    if (decoder->buffer_index + 1 >= decoder->buffer_size) {
        return RI_FAILURE;
    }
    *out_word = decoder->buffer[decoder->buffer_index];
    decoder->buffer_index += 2;
    if (decoder->buffer_index >= decoder->buffer_size) {
        return RI_DONE;
    }
    return RI_CONTINUE;
}



/**
 * Reads the opcode of an instruction and returns the associated instruction tag.
 * @param byte Byte to read the opcode from.
 * @param instruction_tag Instruction type/tag associated with the opcode read.
 * @return SUCCESS if an instruction was found, FAILURE otherwise.
 */
result_t read_opcode(uint8_t byte, instruction_tag_t* out_instruction_tag) {
    int opcodes_count = sizeof(opcodes) / sizeof(opcodes[0]);
    for (int i = 0; i < opcodes_count; i++) {
        if ((byte & opcodes[i].opcode_mask) == opcodes[i].opcode) {
            *out_instruction_tag = i;
            return SUCCESS;
        }
    }
    return FAILURE;
}

result_t decode__move_register_or_memory_to_or_from_register_or_memory(
    decoder_t* decoder, 
    move_register_or_memory_to_or_from_register_or_memory_t *out_move) 
{
    out_move->direction = (decoder->current_byte & 0b00000010) >> 1;
    out_move->wide = decoder->current_byte & 0b00000001;
    if (dcd_read_byte(decoder, (uint8_t*) &decoder->current_byte) == RI_FAILURE) { 
        fprintf(stderr, "decode__move_register_or_memory_to_from_register_or_memory: \
                failed to parse byte 2nd field byte.\n");
        return FAILURE;
    }
    out_move->mod = (decoder->current_byte & 0b11000000) >> 6;
    out_move->reg = (decoder->current_byte & 0b00111000) >> 3;
    out_move->rm = decoder->current_byte & 0b00000111;

    if (out_move->mod == MOD_MEMORY) {
        if (out_move->rm == 0b00000110) {
            if (dcd_read_word(decoder, &out_move->displacement) == RI_FAILURE) { 
                fprintf(stderr, "decode__move_register_or_memory_to_from_register_or_memory: \
                        failed to parse word displacement (rm = 3).\n");
                return FAILURE; 
            }
        }
    } else if (out_move->mod == MOD_MEMORY_8BIT_DISPLACEMENT) {
        if (dcd_read_byte(decoder, (uint8_t*) &out_move->displacement) == RI_FAILURE) { 
            fprintf(stderr, "decode__move_register_or_memory_to_from_register_or_memory: \
                    failed to parse byte displacement.\n");
            return FAILURE;
        }
    } else if (out_move->mod == MOD_MEMORY_16BIT_DISPLACEMENT) {
        if (dcd_read_word(decoder, &out_move->displacement) == RI_FAILURE) { 
            fprintf(stderr, "decode__move_register_or_memory_to_from_register_or_memory: \
                    failed to parse word displacement.\n");
            return FAILURE; 
        }
    } else { // MOD_REGISTER
        // Don't have extra bytes for register to register movs. Nothing to do.
    }

    return SUCCESS;
}

void write__move_register_or_memory_to_from_register_or_memory(
    move_register_or_memory_to_or_from_register_or_memory_t* move, 
    char* buffer, 
    int buffer_size)
{
    if (move->mod == MOD_REGISTER) {
        uint8_t* left = &move->rm;
        uint8_t* right = (uint8_t*)&move->reg;
        if (move->direction == DIR_FROM_REGISTER) {
            left = (uint8_t*)&move->reg;
            right = &move->rm;
        }

        char* left_string = regb_strings[*left];
        char* right_string = regb_strings[*right];

        if (move->wide == WIDE_WORD) {
            left_string = regw_strings[*left];
            right_string = regw_strings[*right];
        }

        snprintf(buffer, buffer_size, "%s %s, %s", 
                instruction_metadata[I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY]
                    .mnemonic,
                left_string,
                right_string);
    } else {
        printf("rm: %d\n", move->rm);
        if (move->direction == DIR_TO_REGISTER) {
            char* reg_string = regb_strings[move->reg];
            if (move->wide == WIDE_WORD) {
                reg_string = regw_strings[move->reg];
            }

            // TODO: replace "???" with memory address as dictated by RM
            snprintf(buffer, buffer_size, "%s %s, %s", 
                        instruction_metadata[I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY]
                            .mnemonic,
                        "[???]",
                        reg_string);
        } else { 
            char* reg_string = regb_strings[move->reg];
            if (move->wide == WIDE_WORD) {
                reg_string = regw_strings[move->reg];
            }
            snprintf(buffer, buffer_size, "%s %s, %s", 
                        instruction_metadata[I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY]
                            .mnemonic,
                        reg_string,
                        "???");
        }
        
        //snprintf(buffer, buffer_size, "NOT IMPLEMENTED!");
    }
}

result_t decode__move_immediate_to_register_or_memory(
    decoder_t* decoder, 
    move_immediate_to_register_or_memory_t* out_move) 
{
    out_move->wide = (decoder->current_byte & 0b00001000) >> 3;
    // TODO
    return FAILURE;
}

result_t decode__move_immediate_to_register(
    decoder_t* decoder, 
    move_immediate_to_register_t* out_move) 
{
    out_move->wide = (decoder->current_byte & 0b00001000) >> 3;
    out_move->reg = decoder->current_byte & 0b00000111;
    if (out_move->wide == WIDE_BYTE) {
        if (dcd_read_byte(decoder, (uint8_t*)&out_move->immediate) == RI_FAILURE) {
            fprintf(stderr, "decode__move_immediate_to_register: failed to parse byte immediate.\n");
            return FAILURE;
        }
    } else { // WIDE_WORD
        if (dcd_read_word(decoder, &out_move->immediate) == RI_FAILURE) {
            fprintf(stderr, "decode__move_immediate_to_register: failed to parse word immediate.\n");
            return FAILURE; 
        }
    }
    return SUCCESS;
}

void write__move_immediate_to_register_or_memory(
    move_immediate_to_register_or_memory_t* move, 
    char* buffer, 
    int buffer_size)
{
    snprintf(buffer, buffer_size, "TODO: I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY!");
}

void write__move_immediate_to_register(
    move_immediate_to_register_t* move, 
    char* buffer, 
    int buffer_size)
{
    if (move->wide == WIDE_BYTE) {
        snprintf(buffer, buffer_size, "%s %s, %d", 
            instruction_metadata[I_MOVE_IMMEDIATE_TO_REGISTER].mnemonic,
            regb_strings[move->reg],
            move->immediate);
    } else { // WIDE_WORD
        snprintf(buffer, buffer_size, "%s %s, %d", 
            instruction_metadata[I_MOVE_IMMEDIATE_TO_REGISTER].mnemonic,
            regw_strings[move->reg],
            move->immediate);
    }
}

void write__instruction(instruction_t* instruction, char* buffer, int buffer_size) {
    switch(instruction->tag) {
        case I_INVALID:
        snprintf(buffer, buffer_size, "INVALID!");
            break;
        case I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY:
            write__move_register_or_memory_to_from_register_or_memory(
                &instruction->data.move_register_or_memory_to_or_from_register_or_memory, 
                buffer, 
                buffer_size
            );
            break;
        case I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY:
            write__move_immediate_to_register_or_memory(
                &instruction->data.move_immediate_to_register_or_memory,
                buffer,
                buffer_size
            );
            break;
        case I_MOVE_IMMEDIATE_TO_REGISTER:
            write__move_immediate_to_register(
                &instruction->data.move_immediate_to_register,
                buffer, 
                buffer_size
            );
            break;
        default:
            snprintf(buffer, buffer_size, "NOT IMPLEMENTED! tag: %d", instruction->tag);
            break;
    }
}

result_iter_t next(decoder_t* decoder) {
    decoder->current_byte = decoder->buffer[decoder->buffer_index];
    printf("next: index: %d\n", decoder->buffer_index);
    decoder->buffer_index += 1;
    if (decoder->buffer_index >= decoder->buffer_size) {
        return RI_DONE;
    }

    printf("next: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(decoder->current_byte));
    instruction_tag_t instruction_tag = 0;
    result_t read_result = read_opcode(decoder->current_byte, &instruction_tag);

    instruction_t* next_instr = &decoder->instructions[decoder->instructions_count];
    decoder->instructions_count += 1;

    next_instr->tag = instruction_tag;
    result_iter_t result = RI_FAILURE;
    if (read_result == SUCCESS) {
        switch(instruction_tag) {
            // MOV
            case I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY:
                result = decode__move_register_or_memory_to_or_from_register_or_memory(decoder, 
                            &next_instr->data.move_register_or_memory_to_or_from_register_or_memory);
                break;
            case I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY:
                result = decode__move_immediate_to_register_or_memory(decoder, 
                            &next_instr->data.move_immediate_to_register_or_memory);
                break;
            case I_MOVE_IMMEDIATE_TO_REGISTER:
                result = decode__move_immediate_to_register(decoder, 
                            &next_instr->data.move_immediate_to_register);
                break;
            case I_MOVE_MEMORY_TO_ACCUMULATOR:
            case I_MOVE_ACCUMULATOR_TO_MEMORY:
            case I_MOVE_REGISTER_OR_MEMORY_TO_SEGMENT_REGISTER:
            case I_MOVE_SEGMENT_REGISTER_TO_REGISTER_OR_MEMORY:
                printf("Not implemented!\n");
                break;
            // PUSH
            // case I_PUSH_REGISTER_OR_MEMORY:
            // case I_PUSH_REGISTER:
            // case I_PUSH_SEGMENT_REGISTER:
            //     printf("Not implemented!\n");
            //     break;
            // XCHNG
            // IN
            // OUT
            // ...
            default:
                printf("Not implemented!\n");
                break;
        }
    }
    if (result == FAILURE) {
        fprintf(stderr, "Failed to parse instruction! decode_result = failure\n");
        return RI_FAILURE;
    }
    return RI_CONTINUE;
}

result_t decode_file(decoder_t* decoder, char* input_path) {
    printf("Starting decode file...\n");
    FILE* file = fopen(input_path, "r");
    
    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    rewind(file);
    decoder->buffer = (uint8_t*) malloc(file_size);
    memset(decoder->buffer, 0, file_size);
    decoder->buffer_index = 0;
    decoder->buffer_size = file_size;
    int read_result = fread(decoder->buffer, 1, file_size, file);
    if (read_result != file_size) {
        fprintf(stderr, "Failed to read file!\n");
        return FAILURE;
    }

    result_t result = decode(decoder);
    free(decoder->buffer);
    return result;
}

result_t decode_chunk(decoder_t* decoder, char* buffer, size_t buffer_size) {
    decoder->buffer = buffer;
    decoder->buffer_index = 0;
    decoder->buffer_size = buffer_size;
    return decode(decoder);
}

result_t decode(decoder_t* decoder) {
    for (int i = 0; i < decoder->buffer_size; i++) {
        printf("[%d] "BYTE_TO_BINARY_PATTERN"\n", i, BYTE_TO_BINARY(decoder->buffer[i]));
    }

    result_t result = next(decoder);
    while(result == RI_CONTINUE) {
        result = next(decoder);
    }

    if (result == RI_DONE) {
        return SUCCESS;
    } else {
        return FAILURE;
    }
}
