
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "shared/include/binary_utilities.h"
//#include "8086/instruction_tags_8086.h"
//#include "shared/include/registers.h"
#include "shared/include/result.h"

#include "libraries/emulate_intel/include/emulate.h"
#include "logger.h"

#include "i386/instruction_tags_i386.h"
#include "i386/decode_i386_common.h"
#include "i386/decode_i386_tag.h"
#include "i386/decode_i386.h"


static result_iter_t emu_i386_decode_next(emulator_i386_t* emulator, char* out_buffer, int* index, size_t out_buffer_size) {
    uint8_t byte1 = emulator->memory[emulator->registers.ip];
    emulator->registers.ip += 1;
    LOGD("ip: %d, byte1: %x", emulator->registers.ip, byte1);

    // If we reach an empty byte, assume we've hit the end of the program.
    if (byte1 == 0x00) {
        return(RI_DONE);
    }

    instruction_tag_i386_t instruction_tag = 0;
    uint8_t byte2 = 0;
    if (emulator->registers.ip < emulator->memory_size) {
        byte2 = emulator->memory[emulator->registers.ip];
    }
    instruction_tag = emu_i386_decode_tag(byte1, byte2);
    emulator->instructions_count += 1;

    emu_result_t result = RI_FAILURE;
    switch(instruction_tag) {
        // ...
        // MARK: LOGIC
        // ...
        // MARK: AND
        // case I_i386_AND_IMMEDIATE:
        // case I_i386_AND_AX:
        case I_i386_AND:
            result = emu_i386_decode_and_write_common_standard_format(emulator,
                byte1, "and", 3, out_buffer, index, out_buffer_size);
            break;
        // ...
        default:
            LOG(LOG_ERROR, "Instruction tag: %d, not implemented", instruction_tag);
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

result_t emu_i386_decode_chunk(
    emulator_i386_t* emulator,
    char* in_buffer,
    size_t in_buffer_size,
    char* out_buffer,
    size_t out_buffer_size)
{
    memcpy(emulator->memory + PROGRAM_START, in_buffer, in_buffer_size);
    emulator->registers.ip = PROGRAM_START;
    return(emu_i386_decode(emulator, out_buffer, out_buffer_size));
}

result_t emu_i386_decode(emulator_i386_t* emulator, char* out_buffer, size_t out_buffer_size) {
    int index = 0;
    result_iter_t result = RI_CONTINUE;
    
    do {
        result = emu_i386_decode_next(emulator, out_buffer, &index, out_buffer_size);
    } while(result == RI_CONTINUE);

    if (result == RI_DONE) {
        return(SUCCESS);
    } else {
        return(FAILURE);
    }
}
