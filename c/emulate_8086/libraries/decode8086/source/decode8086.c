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
#include "libraries/decode8086/include/decode_tag.h"

#include "libraries/decode8086/include/instructions/decode_mov.h"
#include "libraries/decode8086/include/instructions/decode_add.h"
#include "libraries/decode8086/include/instructions/decode_sub.h"
#include "libraries/decode8086/include/instructions/decode_cmp.h"

#include "libraries/decode8086/include/instructions/conditional_jumps/je.h"
#include "libraries/decode8086/include/instructions/decode_conditional_jumps.h"

//#include "decode_tag.c"

/**
 * Initializes the decoder.
 */
void dcd_init(decoder_t* decoder, instruction_t* instructions, size_t instructions_size) {
    decoder->instructions = instructions;
    decoder->instructions_capacity = instructions_size;
    decoder->instructions_count = 0;
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
        // MARK: MOV
        case I_MOVE:
            write_move(
                &instruction->data.move,
                buffer, index, buffer_size);
            break;
        case I_MOVE_IMMEDIATE:
            write__move_immediate_to_register_or_memory(
                &instruction->data.move_immediate_to_register_or_memory,
                buffer, index, buffer_size);
            break;
        case I_MOVE_IMMEDIATE_TO_REGISTER:
            write__move_immediate_to_register(
                &instruction->data.move_immediate_to_register,
                buffer, index, buffer_size);
            break;
        case I_MOVE_TO_AX:
            write__move_memory_to_accumulator(
                &instruction->data.move_memory_to_accumulator,
                buffer, index, buffer_size);
            break;
        case I_MOVE_AX:
            write__move_accumulator_to_memory(
                &instruction->data.move_accumulator_to_memory,
                buffer, index, buffer_size);
            break;
        // TODO: all instructions between MOV and ADD
        // MARK: ADD
        case I_ADD:
            write_add(&instruction->data.add, buffer, index, buffer_size);
            break;
        case I_ADD_IMMEDIATE:
            write_add_immediate(&instruction->data.add_immediate, buffer, index, buffer_size);
            break;
        // TODO: between add and sub
        case I_SUB:
            write_sub(&instruction->data.sub, buffer, index, buffer_size);
            break;
        // TODO: between sub and cmp
        case I_COMPARE:
            write_compare(&instruction->data.compare, buffer, index, buffer_size);
            break;
        // MARK: CONDITIONAL JUMPS
        case I_JUMP_ON_EQUAL:
        case I_JUMP_ON_LESS:
        case I_JUMP_ON_LESS_OR_EQUAL:
        case I_JUMP_ON_BELOW:
        case I_JUMP_ON_BELOW_OR_EQUAL:
        case I_JUMP_ON_PARITY:
        case I_JUMP_ON_OVERLFLOW:
        case I_JUMP_ON_SIGN:
        case I_JUMP_ON_NOT_EQUAL:
        case I_JUMP_ON_GREATER_OR_EQUAL:
        case I_JUMP_ON_GREATER:
        case I_JUMP_ON_ABOVE_OR_EQUAL:
        case I_JUMP_ON_ABOVE:
        case I_JUMP_ON_NOT_PARITY:
        case I_JUMP_ON_NOT_OVERFLOW:
        case I_JUMP_ON_NOT_SIGN:
        case I_LOOP:
        case I_LOOP_WHILE_EQUAL:
        case I_LOOP_WHILE_NOT_EQUAL:
        case I_JUMP_ON_CX_ZERO:
            write_conditional_jump(&instruction->data.conditional_jump, instruction->tag, buffer, index, buffer_size);
            break;

        default:
            snprintf(buffer, buffer_size, "NOT IMPLEMENTED! tag: %d", instruction->tag);
            printf("write NOT IMPLEMENTED! tag: %d\n", instruction->tag);
            break;
    }
}

result_iter_t next(decoder_t* decoder) {
    decoder->current_byte = decoder->buffer[decoder->buffer_index];
    uint8_t byte1 = decoder->current_byte;
    decoder->buffer_index += 1;
    if (decoder->buffer_index >= decoder->buffer_size) {
        return RI_DONE;
    }

    //printf("next: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(byte1));
    instruction_tag_t instruction_tag = 0;
    uint8_t byte2 = 0;
    if (decoder->buffer_index < decoder->buffer_size) {
        byte2 = decoder->buffer[decoder->buffer_index];
    }
    instruction_tag = dcd_decode_tag(byte1, byte2);

    instruction_t* instruction = &decoder->instructions[decoder->instructions_count];
    instruction->tag = instruction_tag;
    decoder->instructions_count += 1;

    decode_result_t result = RI_FAILURE;
    switch(instruction->tag) {
        // TODO: rename this to "expand"?
        // MARK: MOV
        case I_MOVE:
            result = decode_move(decoder, byte1, &instruction->data.move);
            break;
        case I_MOVE_IMMEDIATE:
            result = decode__move_immediate_to_register_or_memory(decoder, byte1,
                &instruction->data.move_immediate_to_register_or_memory);
            break;
        case I_MOVE_IMMEDIATE_TO_REGISTER:
            result = decode__move_immediate_to_register(decoder, byte1,
                &instruction->data.move_immediate_to_register);
            break;
        case I_MOVE_TO_AX:
            result = decode__move_memory_to_accumulator(decoder, byte1,
                &instruction->data.move_memory_to_accumulator);
            break;
        case I_MOVE_AX:
            result = decode__move_accumulator_to_memory(decoder, byte1,
                &instruction->data.move_accumulator_to_memory);
            break;
        case I_MOVE_TO_SEGMENT_REGISTER:
        case I_MOVE_SEGMENT_REGISTER:
            printf("Not implemented!\n");
            result = DR_UNIMPLEMENTED_INSTRUCTION;
            break;
        // PUSH
        // case I_PUSH:
        // case I_PUSH_REGISTER:
        // case I_PUSH_SEGMENT_REGISTER:
        //     printf("Not implemented!\n");
        //     break;
        // XCHNG
        // IN
        // OUT
        // ARITHMETIC
        // MARK: ADD
        case I_ADD:
            result = decode_add(decoder, byte1, &instruction->data.add);
            break;
        case I_ADD_IMMEDIATE:
            result = decode_add_immediate(decoder, byte1, &instruction->data.add_immediate);
            break;
        // ADC
        // ...
        // MARK: SUB
        case I_SUB:
            result = decode_sub(decoder, byte1, &instruction->data.sub);
            break;
        // ...
        // CMP
        case I_COMPARE:
            result = decode_compare(decoder, byte1, &instruction->data.compare);
            break;
        // ...
        // MARK: CONDITIONAL JUMPS
        case I_JUMP_ON_EQUAL:
        case I_JUMP_ON_LESS:
        case I_JUMP_ON_LESS_OR_EQUAL:
        case I_JUMP_ON_BELOW:
        case I_JUMP_ON_BELOW_OR_EQUAL:
        case I_JUMP_ON_PARITY:
        case I_JUMP_ON_OVERLFLOW:
        case I_JUMP_ON_SIGN:
        case I_JUMP_ON_NOT_EQUAL:
        case I_JUMP_ON_GREATER_OR_EQUAL:
        case I_JUMP_ON_GREATER:
        case I_JUMP_ON_ABOVE_OR_EQUAL:
        case I_JUMP_ON_ABOVE:
        case I_JUMP_ON_NOT_PARITY:
        case I_JUMP_ON_NOT_OVERFLOW:
        case I_JUMP_ON_NOT_SIGN:
        case I_LOOP:
        case I_LOOP_WHILE_EQUAL:
        case I_LOOP_WHILE_NOT_EQUAL:
        case I_JUMP_ON_CX_ZERO:
            result = decode_conditional_jump2(decoder, instruction->tag, byte1, &instruction->data.conditional_jump);
            break;
        case I_INTERRUPT_TYPE_SPECIFIED:
        case I_INTERRUPT_TYPE_3:
        case I_INTERRUPT_ON_OVERFLOW:
        case I_INTERRUPT_RETURN:
            printf("Not implemented! %d\n", instruction_tag);
            result = DR_UNIMPLEMENTED_INSTRUCTION;
            break;
        case I_CLEAR_CARRY:
        case I_COMPLEMENT_CARRY:
        case I_SET_CARRY:
        case I_CLEAR_DIRECTION:
        case I_SET_DIRECTION:
        case I_CLEAR_INTERRUPT:
        case I_SET_INTERRUPT:
        case I_HALT:
        case I_WAIT:

            // TODO: "decode_command"
            printf("Not implemented! %d\n", instruction_tag);
            result = DR_UNIMPLEMENTED_INSTRUCTION;
            break;
        case I_ESCAPE:
            // TODO: read 4 bytes
            printf("Not implemented! %d\n", instruction_tag);
            result = DR_UNIMPLEMENTED_INSTRUCTION;
            break;
        case I_LOCK:
        case I_SEGMENT:
            // TODO: "decode_command"
            printf("Not implemented! %d\n", instruction_tag);
            result = DR_UNIMPLEMENTED_INSTRUCTION;
            break;
        // ...
        default:
            printf("Not implemented! %d\n", instruction_tag);
            result = DR_UNIMPLEMENTED_INSTRUCTION;
            break;

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
