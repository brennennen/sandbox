/**
 *
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/registers.h"
#include "shared/include/result.h"

#include "libraries/emulate8086/include/emulate8086.h"
#include "libraries/emulate8086/include/emu_registers.h"
#include "libraries/emulate8086/include/decode_utils.h"
#include "libraries/emulate8086/include/decode_tag.h"

#include "libraries/emulate8086/include/instructions/mov.h"
#include "libraries/emulate8086/include/instructions/push.h"
#include "libraries/emulate8086/include/instructions/pop.h"
#include "libraries/emulate8086/include/instructions/add.h"
#include "libraries/emulate8086/include/instructions/arithmetic/inc.h"
#include "libraries/emulate8086/include/instructions/sub.h"
#include "libraries/emulate8086/include/instructions/cmp.h"

#include "libraries/emulate8086/include/instructions/conditional_jumps.h"
#include "libraries/emulate8086/include/instructions/processor_control/clc.h"
#include "libraries/emulate8086/include/instructions/processor_control/cmc.h"
#include "libraries/emulate8086/include/instructions/processor_control/stc.h"

//#include "decode_tag.c"

/**
 * Initializes the decoder.
 */
void emu_init(emulator_t* emulator) {

}

result_iter_t emu_decode_next(emulator_t* decoder, char* out_buffer, int* index, size_t out_buffer_size) {
    decoder->current_byte = decoder->buffer[decoder->buffer_index];
    uint8_t byte1 = decoder->current_byte;
    decoder->buffer_index += 1;
    if (decoder->buffer_index > decoder->buffer_size) {
        return RI_DONE;
    }

    //printf("next: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(byte1));
    instruction_tag_t instruction_tag = 0;
    uint8_t byte2 = 0;
    if (decoder->buffer_index < decoder->buffer_size) {
        byte2 = decoder->buffer[decoder->buffer_index];
    }
    instruction_tag = dcd_decode_tag(byte1, byte2);

    //instruction_t* instruction = &decoder->instructions[decoder->instructions_count];
    //instruction->tag = instruction_tag;
    decoder->instructions_count += 1;

    emu_result_t result = RI_FAILURE;
    switch(instruction_tag) {
        // TODO: rename this to "expand"?
        // MARK: MOV
        case I_MOVE:
            result = decode_move(decoder, byte1, out_buffer, index, out_buffer_size);
            break;
        case I_MOVE_IMMEDIATE:
            result = decode_move_immediate(decoder, byte1, out_buffer, index, out_buffer_size);
            break;
        case I_MOVE_IMMEDIATE_TO_REGISTER:
            result = decode_move_immediate_to_register(decoder, byte1, out_buffer, index, out_buffer_size);
            break;
        case I_MOVE_TO_AX:
            result = decode_move_to_ax(decoder, byte1, out_buffer, index, out_buffer_size);
            break;
        case I_MOVE_AX:
            result = decode_move_ax(decoder, byte1, out_buffer, index, out_buffer_size);
            break;
        case I_MOVE_TO_SEGMENT_REGISTER:
        case I_MOVE_SEGMENT_REGISTER:
            printf("Not implemented!\n");
            result = ER_UNIMPLEMENTED_INSTRUCTION;
            break;
        // MARK: PUSH
        // case I_PUSH:
        //     printf("Not implemented!\n");
        //     break;
        case I_PUSH_REGISTER:
            result = decode_push_register(decoder, byte1, out_buffer, index, out_buffer_size);
            break;
        // case I_PUSH_SEGMENT_REGISTER:
        //     printf("Not implemented!\n");
        //     break;
        // MARK: POP
        // case I_POP:
        case I_POP_REGISTER:
            result = decode_pop_register(decoder, byte1, out_buffer, index, out_buffer_size);
            break;
        // case I_POP_SEGMENT_REGISTER:
        // XCHNG
        // IN
        // OUT
        // ARITHMETIC
        // MARK: ADD
        case I_ADD:
            result = decode_add(decoder, byte1, out_buffer, index, out_buffer_size);
            break;
        case I_ADD_IMMEDIATE:
            result = decode_add_immediate(decoder, byte1, out_buffer, index, out_buffer_size);
            break;
        // ADC
        // MARK: INC
        case I_INC:
            result = decode_inc(decoder, byte1, out_buffer, index, out_buffer_size);
            break;
        case I_INC_REGISTER:
            result = decode_inc_register(decoder, byte1, out_buffer, index, out_buffer_size);
            break;
        // ...
        // MARK: SUB
        case I_SUB:
            result = decode_sub(decoder, byte1, out_buffer, index, out_buffer_size);
            break;
        case I_SUB_IMMEDIATE:
            result = decode_sub_immediate(decoder, byte1, out_buffer, index, out_buffer_size);
            break;
        // ...
        // CMP
        case I_COMPARE:
            result = decode_compare(decoder, byte1, out_buffer, index, out_buffer_size);
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
            result = decode_conditional_jump2(decoder, instruction_tag, byte1, out_buffer, index, out_buffer_size);
            break;
        case I_INTERRUPT_TYPE_SPECIFIED:
        case I_INTERRUPT_TYPE_3:
        case I_INTERRUPT_ON_OVERFLOW:
        case I_INTERRUPT_RETURN:
            printf("Not implemented! %d\n", instruction_tag);
            result = ER_UNIMPLEMENTED_INSTRUCTION;
            break;
        case I_CLEAR_CARRY:
            result = decode_clc(decoder, byte1, out_buffer, index, out_buffer_size);
            break;
        case I_COMPLEMENT_CARRY:
            result = decode_cmc(decoder, byte1, out_buffer, index, out_buffer_size);
            break;
        case I_SET_CARRY:
            result = decode_stc(decoder, byte1, out_buffer, index, out_buffer_size);
            break;
        case I_CLEAR_DIRECTION:
        case I_SET_DIRECTION:
        case I_CLEAR_INTERRUPT:
        case I_SET_INTERRUPT:
        case I_HALT:
        case I_WAIT:
            //instruction->data.byte.byte = byte1;
            // TODO: "decode_command"
            printf("Not implemented! %d\n", instruction_tag);
            result = ER_UNIMPLEMENTED_INSTRUCTION;
            break;
        case I_ESCAPE:
            // TODO: read 4 bytes
            printf("Not implemented! %d\n", instruction_tag);
            result = ER_UNIMPLEMENTED_INSTRUCTION;
            break;
        case I_LOCK:
        case I_SEGMENT:
            // TODO: "decode_command"
            printf("Not implemented! %d\n", instruction_tag);
            result = ER_UNIMPLEMENTED_INSTRUCTION;
            break;
        // ...
        default:
            printf("Not implemented! %d\n", instruction_tag);
            result = ER_UNIMPLEMENTED_INSTRUCTION;
            break;

    }
    if (result != ER_SUCCESS) {
        fprintf(stderr, "Failed to parse instruction! decode_result = %s (%d)\n", emulate_result_strings[result], result);
        return RI_FAILURE;
    }
    return RI_CONTINUE;
}

result_t emu_decode_file(
    emulator_t* emulator,
    char* input_path,
    char* out_buffer,
    size_t out_buffer_size)
{
    printf("Starting decode file...\n");
    FILE* file = fopen(input_path, "r");

    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    rewind(file);
    emulator->buffer = (uint8_t*) malloc(file_size);
    memset(emulator->buffer, 0, file_size);
    emulator->buffer_index = 0;
    emulator->buffer_size = file_size;
    int read_result = fread(emulator->buffer, 1, file_size, file);
    if (read_result != file_size) {
        fprintf(stderr, "Failed to read file!\n");
        return FAILURE;
    }

    result_t result = emu_decode(emulator, out_buffer, out_buffer_size);
    free(emulator->buffer);
    return result;
}

result_t emu_decode_chunk(
    emulator_t* emulator,
    char* in_buffer,
    size_t in_buffer_size,
    char* out_buffer,
    size_t out_buffer_size)
{
    emulator->buffer = in_buffer;
    emulator->buffer_index = 0;
    emulator->buffer_size = in_buffer_size;
    return emu_decode(emulator, out_buffer, out_buffer_size);
}

result_t emu_decode(emulator_t* emulator, char* out_buffer, size_t out_buffer_size) {
    for (int i = 0; i < emulator->buffer_size; i++) {
        printf("[%d] "BYTE_TO_BINARY_PATTERN"\n", i, BYTE_TO_BINARY(emulator->buffer[i]));
    }

    int index = 0;
    result_t result = emu_decode_next(emulator, out_buffer, &index, out_buffer_size);
    while(result == RI_CONTINUE) {
        result = emu_decode_next(emulator, out_buffer, &index, out_buffer_size);
    }

    if (result == RI_DONE) {
        return SUCCESS;
    } else {
        return FAILURE;
    }
}

result_iter_t emu_next(emulator_t* emulator) {
    emulator->current_byte = emulator->buffer[emulator->buffer_index];
    uint8_t byte1 = emulator->current_byte;
    emulator->buffer_index += 1;
    if (emulator->buffer_index > emulator->buffer_size) {
        return RI_DONE;
    }

    //printf("next: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(byte1));
    instruction_tag_t instruction_tag = 0;
    uint8_t byte2 = 0;
    if (emulator->buffer_index < emulator->buffer_size) {
        byte2 = emulator->buffer[emulator->buffer_index];
    }
    instruction_tag = dcd_decode_tag(byte1, byte2);

    //instruction_t* instruction = &emulator->instructions[emulator->instructions_count];
    //instruction->tag = instruction_tag;
    emulator->instructions_count += 1;

    emu_result_t result = RI_FAILURE;
    switch(instruction_tag) {
    // MARK: MOV
    case I_MOVE:
        result = emu_move(emulator, byte1);
        break;
    case I_MOVE_IMMEDIATE:
        result = emu_move_immediate(emulator, byte1);
        break;
    case I_MOVE_IMMEDIATE_TO_REGISTER:
        result = emu_move_immediate_to_register(emulator, byte1);
        break;
    case I_MOVE_TO_AX:
        result = emu_move_to_ax(emulator, byte1);
        break;
    case I_MOVE_AX:
        result = emu_move_ax(emulator, byte1);
        break;
    case I_MOVE_TO_SEGMENT_REGISTER:
    case I_MOVE_SEGMENT_REGISTER:
        printf("Not implemented!\n");
        result = ER_UNIMPLEMENTED_INSTRUCTION;
        break;
    // PUSH
    // case I_PUSH:
    case I_PUSH_REGISTER:
        result = emu_push_register(emulator, byte1);
        break;
    // case I_PUSH_SEGMENT_REGISTER:
    //     printf("Not implemented!\n");
    //     break;
    // MARK: POP
    // case I_POP:
    case I_POP_REGISTER:
        result = emu_pop_register(emulator, byte1);
        break;
    // case I_POP_SEGMENT_REGISTER:
    // XCHNG
    // IN
    // OUT
    // ARITHMETIC
    // MARK: ADD
    case I_ADD:
        result = emu_add(emulator, byte1);
        break;
    case I_ADD_IMMEDIATE:
        result = emu_add_immediate(emulator, byte1);
        break;
    // ADC
    // MARK: INC
    case I_INC:
        result = emu_inc(emulator, byte1);
        break;
    case I_INC_REGISTER:
        result = emu_inc_register(emulator, byte1);
        break;
    // ...
    // MARK: SUB
    case I_SUB:
        result = emu_sub(emulator, byte1);
        break;
    case I_SUB_IMMEDIATE:
        result = emu_sub_immediate(emulator, byte1);
        break;
    // ...
    // CMP
    case I_COMPARE:
        result = emu_compare(emulator, byte1);
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
        result = emu_conditional_jump(emulator, instruction_tag, byte1);
        break;
    case I_INTERRUPT_TYPE_SPECIFIED:
    case I_INTERRUPT_TYPE_3:
    case I_INTERRUPT_ON_OVERFLOW:
    case I_INTERRUPT_RETURN:
        printf("Not implemented! %d\n", instruction_tag);
        result = ER_UNIMPLEMENTED_INSTRUCTION;
        break;
    case I_CLEAR_CARRY:
        result = emu_clc(emulator, byte1);
        break;
    case I_COMPLEMENT_CARRY:
        result = emu_cmc(emulator, byte1);
        break;
    case I_SET_CARRY:
        result = emu_stc(emulator, byte1);
        break;
    case I_CLEAR_DIRECTION:
    case I_SET_DIRECTION:
    case I_CLEAR_INTERRUPT:
    case I_SET_INTERRUPT:
    case I_HALT:
    case I_WAIT:
        //instruction->data.byte.byte = byte1;
        // TODO: "decode_command"
        printf("Not implemented! %d\n", instruction_tag);
        result = ER_UNIMPLEMENTED_INSTRUCTION;
        break;
    case I_ESCAPE:
        // TODO: read 4 bytes
        printf("Not implemented! %d\n", instruction_tag);
        result = ER_UNIMPLEMENTED_INSTRUCTION;
        break;
    case I_LOCK:
    case I_SEGMENT:
        // TODO: "decode_command"
        printf("Not implemented! %d\n", instruction_tag);
        result = ER_UNIMPLEMENTED_INSTRUCTION;
        break;
    // ...
    default:
        printf("Not implemented! %d\n", instruction_tag);
        result = ER_UNIMPLEMENTED_INSTRUCTION;
        break;
    }
    if (result != ER_SUCCESS) {
        fprintf(stderr, "Failed to parse instruction! decode_result = %s (%d)\n", emulate_result_strings[result], result);
        return RI_FAILURE;
    }
    return RI_CONTINUE;
}

result_t emu_emulate_file(emulator_t* emulator, char* input_path) {
    printf("Starting emulate file...\n");
    FILE* file = fopen(input_path, "r");

    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    rewind(file);
    emulator->buffer = (uint8_t*) malloc(file_size);
    memset(emulator->buffer, 0, file_size);
    emulator->buffer_index = 0;
    emulator->buffer_size = file_size;
    int read_result = fread(emulator->buffer, 1, file_size, file);
    if (read_result != file_size) {
        fprintf(stderr, "Failed to read file!\n");
        return FAILURE;
    }

    result_t result = emu_emulate(emulator);
    free(emulator->buffer);
    return result;
}

result_t emu_emulate_chunk(
    emulator_t* emulator,
    char* in_buffer,
    size_t in_buffer_size
) {
    emulator->buffer = in_buffer;
    emulator->buffer_index = 0;
    emulator->buffer_size = in_buffer_size;
    return emu_emulate(emulator);
}

result_t emu_emulate(emulator_t* emulator) {
    for (int i = 0; i < emulator->buffer_size; i++) {
        printf("[%d] "BYTE_TO_BINARY_PATTERN"\n", i, BYTE_TO_BINARY(emulator->buffer[i]));
    }

    result_t result = emu_next(emulator);
    while(result == RI_CONTINUE) {
        result = emu_next(emulator);
    }

    if (result == RI_DONE) {
        return SUCCESS;
    } else {
        return FAILURE;
    }
}
