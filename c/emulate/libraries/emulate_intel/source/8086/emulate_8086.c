#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/registers.h"
#include "shared/include/result.h"

#include "libraries/emulate_intel/include/emulate.h"
#include "libraries/emulate_intel/include/emu_registers.h"
#include "libraries/emulate_intel/include/decode_utils.h"
#include "libraries/emulate_intel/include/decode_tag.h"
#include "libraries/emulate_intel/include/logger.h"

#include "libraries/emulate_intel/include/instructions/data_transfer/mov.h"
#include "libraries/emulate_intel/include/instructions/data_transfer/push.h"
#include "libraries/emulate_intel/include/instructions/data_transfer/pop.h"
#include "libraries/emulate_intel/include/instructions/data_transfer/xchg.h"

#include "libraries/emulate_intel/include/instructions/arithmetic/add.h"
#include "libraries/emulate_intel/include/instructions/arithmetic/inc.h"
#include "libraries/emulate_intel/include/instructions/arithmetic/sub.h"
#include "libraries/emulate_intel/include/instructions/arithmetic/cmp.h"

#include "libraries/emulate_intel/include/instructions/logic/not.h"
#include "libraries/emulate_intel/include/instructions/logic/and.h"

#include "libraries/emulate_intel/include/instructions/conditional_jumps.h"

#include "libraries/emulate_intel/include/instructions/processor_control/clc.h"
#include "libraries/emulate_intel/include/instructions/processor_control/cmc.h"
#include "libraries/emulate_intel/include/instructions/processor_control/stc.h"

static result_iter_t emu_8086_next(emulator_t* emulator) {
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

    instruction_tag_t instruction_tag = 0;
    uint8_t byte2 = 0;
    if (emulator->registers.ip < emulator->memory_size) {
        byte2 = emulator->memory[emulator->registers.ip];
    }

    instruction_tag = dcd_decode_tag(byte1, byte2);
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

result_t emu_8086_emulate(emulator_t* emulator) {
    result_t result = emu_8086_next(emulator);
    while(result == RI_CONTINUE) {
        result = emu_8086_next(emulator);
    }

    if (result == RI_DONE) {
        return SUCCESS;
    } else {
        return FAILURE;
    }
}
