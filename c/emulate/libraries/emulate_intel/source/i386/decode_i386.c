
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


static result_iter_t emu_i386_decode_next(emulator_t* decoder, char* out_buffer, int* index, size_t out_buffer_size) {
    uint8_t byte1 = decoder->memory[decoder->registers.ip];
    decoder->registers.ip += 1;
    LOGD("ip: %d, byte1: %x", decoder->registers.ip, byte1);

    // If we reach an empty byte, assume we've hit the end of the program.
    if (byte1 == 0x00) {
        return(RI_DONE);
    }

    instruction_tag_t instruction_tag = 0;
    uint8_t byte2 = 0;
    if (decoder->registers.ip < decoder->memory_size) {
        byte2 = decoder->memory[decoder->registers.ip];
    }
    instruction_tag = dcd_decode_tag(byte1, byte2);
    decoder->instructions_count += 1;

    emu_result_t result = RI_FAILURE;
    switch(instruction_tag) {
        // ...
        // MARK: LOGIC
        // ...
        // MARK: AND
        case I_AND:
            //result = decode_i386_and(decoder, byte1, out_buffer, index, out_buffer_size);
            break;
        // ...
        default:
            printf("Not implemented! %d\n", instruction_tag);
            result = ER_UNIMPLEMENTED_INSTRUCTION;
            break;

    }
    if (result != ER_SUCCESS) {
        fprintf(stderr, "Failed to parse instruction! decode_result = %s (%d)\n", emulate_result_strings[result], result);
        return(RI_FAILURE);
    }

    snprintf(out_buffer + *index, out_buffer_size - *index, "\n");
    *index += 1;

    return(RI_CONTINUE);
}

result_t emu_i386_decode(emulator_t* emulator, char* out_buffer, size_t out_buffer_size) {
    int index = 0;
    result_t result = emu_i386_decode_next(emulator, out_buffer, &index, out_buffer_size);
    while(result == RI_CONTINUE) {
        result = emu_i386_decode_next(emulator, out_buffer, &index, out_buffer_size);
    }

    if (result == RI_DONE) {
        return(SUCCESS);
    } else {
        return(FAILURE);
    }
}
