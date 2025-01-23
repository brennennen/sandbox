/**
 * 
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"

#include "libraries/decode8086/include/decode8086.h"
#include "libraries/decode8086/include/decode_utils.h"
#include "libraries/decode8086/include/decode_mov.h"

/**
 * Initializes the decoder.
 */
void dcd_init(decoder_t* decoder, instruction_t* instructions, size_t instructions_size) {
    decoder->instructions = instructions;
    decoder->instructions_capacity = instructions_size;
    decoder->instructions_count = 0;
}

/**
 * Reads the opcode of an instruction and returns the associated instruction tag.
 * @param byte Byte to read the opcode from.
 * @param instruction_tag Instruction type/tag associated with the opcode read.
 * @return DR_SUCCESS if an instruction was found for the specified opcode, DR_UNKNOWN_OPCODE otherwise.
 */
decode_result_t dcd_read_opcode(uint8_t byte, instruction_tag_t* instruction_tag) {
    int opcodes_count = sizeof(opcodes) / sizeof(opcodes[0]);
    for (int i = 0; i < opcodes_count; i++) {
        if ((byte & opcodes[i].opcode_mask) == opcodes[i].opcode) {
            *instruction_tag = i;
            return DR_SUCCESS;
        }
    }
    return DR_UNKNOWN_OPCODE;
}

void dcd_write_all_assembly(instruction_t* instructions, size_t instructions_count, 
                            char* buffer, size_t buffer_size) {
    int index = 0;
    //int written = snprintf(buffer, buffer_size, "bits 16\n");
    //index += written;
    for (int i = 0; i < instructions_count; i++) {
        dcd_write_assembly_instruction(&instructions[i], buffer, &index, buffer_size);
        snprintf(buffer + index, buffer_size - index, "\n");
        index += 1;
    }
}

void dcd_write_assembly_instruction(instruction_t* instruction, char* buffer, int* index, size_t buffer_size) {
    switch(instruction->tag) {
        case I_INVALID:
            int written = snprintf((buffer + *index), (buffer_size - *index), "INVALID!");
            *index += written;
            break;
        case I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY:
            write__move_register_or_memory_to_or_from_register_or_memory(
                &instruction->data.move_register_or_memory_to_or_from_register_or_memory, 
                buffer, index, buffer_size);
            break;
        case I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY:
            write__move_immediate_to_register_or_memory(
                &instruction->data.move_immediate_to_register_or_memory, 
                buffer, index, buffer_size);
            break;
        case I_MOVE_IMMEDIATE_TO_REGISTER:
            write__move_immediate_to_register(
                &instruction->data.move_immediate_to_register, 
                buffer, index, buffer_size);
            break;
        case I_MOVE_MEMORY_TO_ACCUMULATOR:
            write__move_memory_to_accumulator(instruction, buffer, index, buffer_size);
            break;
        default:
            snprintf(buffer, buffer_size, "NOT IMPLEMENTED! tag: %d", instruction->tag);
            break;
    }
}

result_iter_t next(decoder_t* decoder) {
    decoder->current_byte = decoder->buffer[decoder->buffer_index];
    uint8_t current_byte = decoder->current_byte;
    printf("next: index: %d\n", decoder->buffer_index);
    decoder->buffer_index += 1;
    if (decoder->buffer_index >= decoder->buffer_size) {
        return RI_DONE;
    }

    printf("next: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(current_byte));
    instruction_tag_t instruction_tag = 0;
    result_t read_result = dcd_read_opcode(decoder->current_byte, &instruction_tag);
    
    instruction_t* instruction = &decoder->instructions[decoder->instructions_count];
    instruction->tag = instruction_tag;
    decoder->instructions_count += 1;

    decode_result_t result = RI_FAILURE;
    if (read_result == SUCCESS) {
        switch(instruction->tag) {
            // MOV
            case I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY:                
                result = decode__move_register_or_memory_to_or_from_register_or_memory(decoder, 
                    current_byte, &instruction->data.move_register_or_memory_to_or_from_register_or_memory);
                break;
            case I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY:
                result = decode__move_immediate_to_register_or_memory(decoder, 
                    current_byte, &instruction->data.move_immediate_to_register_or_memory);
                break;
            case I_MOVE_IMMEDIATE_TO_REGISTER:
                result = decode__move_immediate_to_register(decoder, 
                    current_byte, &instruction->data.move_immediate_to_register);
                break;
            case I_MOVE_MEMORY_TO_ACCUMULATOR:
                result = decode__move_memory_to_accumulator(decoder, 
                current_byte, &instruction->data.move_memory_to_accumulator);
                break;
            case I_MOVE_ACCUMULATOR_TO_MEMORY:
            case I_MOVE_REGISTER_OR_MEMORY_TO_SEGMENT_REGISTER:
            case I_MOVE_SEGMENT_REGISTER_TO_REGISTER_OR_MEMORY:
                printf("Not implemented!\n");
                result = DR_UNIMPLEMENTED_INSTRUCTION;
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
                result = DR_UNIMPLEMENTED_INSTRUCTION;
                break;
        }
    }
    if (result != DR_SUCCESS) {
        fprintf(stderr, "Failed to parse instruction! decode_result = %s (%d)\n", decode_result_strings[result], result);
        return RI_FAILURE;
    }
    return RI_CONTINUE;
}

result_t dcd_decode_file(decoder_t* decoder, char* input_path) {
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

    result_t result = dcd_decode(decoder);
    free(decoder->buffer);
    return result;
}

result_t dcd_decode_chunk(decoder_t* decoder, char* buffer, size_t buffer_size) {
    decoder->buffer = buffer;
    decoder->buffer_index = 0;
    decoder->buffer_size = buffer_size;
    return dcd_decode(decoder);
}

result_t dcd_decode(decoder_t* decoder) {
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
