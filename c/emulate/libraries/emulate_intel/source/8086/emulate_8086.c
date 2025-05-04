#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/result.h"

#include "logger.h"
#include "emulate.h"
#include "8086/emu_8086_registers.h"
#include "8086/decode_8086_utils.h"
#include "8086/decode_8086_tag.h"


#include "8086/instructions/data_transfer/mov.h"
#include "8086/instructions/data_transfer/push.h"
#include "8086/instructions/data_transfer/pop.h"
#include "8086/instructions/data_transfer/xchg.h"

#include "8086/instructions/arithmetic/add.h"
#include "8086/instructions/arithmetic/inc.h"
#include "8086/instructions/arithmetic/sub.h"
#include "8086/instructions/arithmetic/cmp.h"

#include "8086/instructions/logic/not.h"
#include "8086/instructions/logic/and.h"

#include "8086/instructions/conditional_jumps.h"

#include "8086/instructions/processor_control/clc.h"
#include "8086/instructions/processor_control/cmc.h"
#include "8086/instructions/processor_control/stc.h"

emu_result_t emu_8086_init(emulator_8086_t* emulator) {
    emulator->stack_size = STACK_SIZE; // using a size here in case i want to make this dynamic/resizable later.
    emulator->stack_top = 0;
    emulator->memory_size = MEMORY_SIZE;
    memset(emulator->stack, 0, emulator->stack_size);
    return(ER_SUCCESS);
}

emu_result_t emu_memory_set_byte(emulator_8086_t* emulator, uint32_t address, uint8_t value) {
    if (address < emulator->memory_size) {
        emulator->memory[address] = value;
        return(ER_SUCCESS);
    }
    return(ER_FAILURE);
}

emu_result_t emu_memory_set_uint16(emulator_8086_t* emulator, uint32_t address, uint16_t value) {
    if (address + 1 < emulator->memory_size) {
        memcpy(&emulator->memory[address], &value, 2);
        return(ER_SUCCESS);
    }
    return(ER_FAILURE);
}

emu_result_t emu_memory_get_byte(emulator_8086_t* emulator, uint32_t address, uint8_t* out_value) {
    if (address < emulator->memory_size) {
        *out_value = emulator->memory[address];
        return(ER_SUCCESS);
    }
    return(ER_FAILURE);
}

emu_result_t emu_memory_get_uint16(emulator_8086_t* emulator, uint32_t address, uint16_t* out_value) {
    if (address + 1 < emulator->memory_size) {
        memcpy((uint8_t*)out_value, &emulator->memory[address], 2);
        return(ER_SUCCESS);
    }
    return(ER_FAILURE);
}

static result_iter_t emu_8086_next(emulator_8086_t* emulator) {
    uint8_t byte1 = emulator->memory[emulator->registers.ip];
    emulator->registers.ip += 1;

    if (byte1 == 0x00) {
        LOGD("hit 0x00, assuming this is end of program. ip: %d", emulator->registers.ip);
        return RI_DONE;
    }

    if (emulator->instructions_count >= 8192) {
        printf("hit sentinel limit! probably in an infinite loop!\n");
        return RI_DONE;
    }

    instruction_tag_8086_t instruction_tag = 0;
    uint8_t byte2 = 0;
    if (emulator->registers.ip < emulator->memory_size) {
        byte2 = emulator->memory[emulator->registers.ip];
    }

    instruction_tag = emu_8086_decode_tag(byte1, byte2);
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
    // MARK: XCHG
    case I_EXCHANGE:
        result = emu_exchange(emulator, byte1);
        break;
    case I_EXCHANGE_AX:
        result = emu_exchange_ax(emulator, byte1);
        break;
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
    // MARK: LOGIC
    case I_NOT:
        result = emu_not(emulator, byte1);
        break;
    // ...
    // MARK: AND
    case I_AND:
        result = emu_and(emulator, byte1);
        break;
    case I_AND_IMMEDIATE:
        result = emu_and_immediate(emulator, byte1);
        break;
    case I_AND_IMMEDIATE_TO_AX:
        result = emu_and_immediate_to_ax(emulator, byte1);
        break;
    // ...
    // MARK: CONDITIONAL JUMPS
    // case I_JUMP_ON_EQUAL:
    // case I_JUMP_ON_LESS:
    // case I_JUMP_ON_LESS_OR_EQUAL:
    // case I_JUMP_ON_BELOW:
    // case I_JUMP_ON_BELOW_OR_EQUAL:
    // case I_JUMP_ON_PARITY:
    // case I_JUMP_ON_OVERLFLOW:
    // case I_JUMP_ON_SIGN:
    case I_JUMP_ON_NOT_EQUAL:
        result = emu_jne(emulator, byte1);
        break;
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

result_t emu_8086_emulate_file(emulator_8086_t* emulator, char* input_path) {
    LOG(LOG_INFO, "Starting emulate file: %s\n", input_path);
    FILE* file = fopen(input_path, "r");

    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    rewind(file);
    int read_result = fread(emulator->memory + PROGRAM_START, 1, file_size, file);
    if (read_result != file_size) {
        fprintf(stderr, "Failed to read file!\n");
        return FAILURE;
    }
    printf("byte1: %02X\n", emulator->memory[PROGRAM_START]);
    emulator->registers.ip = PROGRAM_START;
    LOGMEM(emulator->memory, emulator->registers.ip, sizeof(emulator->memory),
        "emulator->memory[emulator->registers->ip]");
    result_t result = emu_8086_emulate(emulator);
    return result;
}

result_t emu_8086_emulate_chunk(
    emulator_8086_t* emulator,
    char* in_buffer,
    size_t in_buffer_size
) {
    memcpy(emulator->memory + PROGRAM_START, in_buffer, in_buffer_size);
    emulator->registers.ip = PROGRAM_START;
    // LOGD("ip: %d", emulator->registers.ip);
    // LOGMEM(emulator->memory, emulator->registers.ip, sizeof(emulator->memory),
    //     "emulator->memory[emulator->registers->ip]");
    emulator->memory[emulator->registers.ip + in_buffer_size] = 0x00;
    emulator->memory[emulator->registers.ip + in_buffer_size + 1] = 0x00;

    return emu_8086_emulate(emulator);
}

result_t emu_8086_emulate(emulator_8086_t* emulator) {
    result_iter_t result = RI_CONTINUE;
    do {
        result = emu_8086_next(emulator);
    } while (result == RI_CONTINUE);

    if (result == RI_DONE) {
        return(SUCCESS);
    } else {
        return(FAILURE);
    }
}
