
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "shared/include/result.h"

#include "logger.h"

#include "a64/emulate_a64.h"
#include "a64/instructions_a64.h"
#include "a64/emu_a64_decode_instruction.h"

#include "a64/instructions/arithmetic/add.h"



emu_result_t emu_a64_init(emulator_a64_t* emulator) {
    emulator->memory_size = MEMORY_SIZE;
    return(ER_SUCCESS);
}

emu_result_t emu_a64_read_m32(emulator_a64_t* emulator, uint32_t* out_data) {
    if (emulator->registers.r15.x + 1 >= emulator->memory_size) {
        LOG(LOG_ERROR, "read m32: ER_OUT_OF_BOUNDS. ip (+ read size): (%d + 4) >= memory size: %d\n",
            emulator->registers.r15.x, emulator->memory_size);
        return(ER_OUT_OF_BOUNDS);
    }
    *out_data = (emulator->memory[emulator->registers.r15.x] << 24)
        | (emulator->memory[emulator->registers.r15.x + 1] << 16)
        | (emulator->memory[emulator->registers.r15.x + 2] << 8)
        | (emulator->memory[emulator->registers.r15.x + 3]);
    emulator->registers.r15.x += 4;
    return(ER_SUCCESS);
}



static result_iter_t emu_a64_decode_next(
    emulator_a64_t* emulator,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
) {
    uint32_t raw_instruction = 0;
    emu_result_t read_result = emu_a64_read_m32(emulator, &raw_instruction);
    LOGD("ip: %d, raw_instruction: %x", emulator->registers.r15.x - 4, raw_instruction);

    // If we reach an empty byte, assume we've hit the end of the program.
    if (raw_instruction == 0x00) {
        return RI_DONE;
    }

    instruction_tag_a64_t instruction_tag = I_INVALID;
    instruction_tag = emu_a64_decode_instruction_tag(raw_instruction);
    emulator->instructions_count += 1;

    emu_result_t result = RI_FAILURE;
    switch(instruction_tag) {
        // ...
        case I_A64_ADD_IMMEDIATE: {
            result = emu_a64_decode_add_immediate(emulator, raw_instruction, out_buffer, index, out_buffer_size);
            break;
        }
        // ...
    }
    if (result != ER_SUCCESS) {
        fprintf(stderr, "Failed to parse instruction! decode_result = %s (%d)\n", emulate_result_strings[result], result);
        return(RI_FAILURE);
    }

    snprintf(out_buffer + *index, out_buffer_size - *index, "\n");
    *index += 1;

    return(RI_CONTINUE);
}

result_t emu_a64_decode_file(
    emulator_a64_t* emulator,
    char* input_path, char*
    out_buffer,
    size_t out_buffer_size
) {
    // TODO
    return(FAILURE);
}

result_t emu_a64_decode_chunk(
    emulator_a64_t* emulator,
    char* in_buffer,
    size_t in_buffer_size,
    char* out_buffer,
    size_t out_buffer_size
) {
    memcpy(emulator->memory + PROGRAM_START, in_buffer, in_buffer_size);
    emulator->registers.r15.x = PROGRAM_START;
    return emu_a64_decode(emulator, out_buffer, out_buffer_size);
    return(FAILURE);
}

result_t emu_a64_decode(
    emulator_a64_t* emulator,
    char* out_buffer,
    size_t out_buffer_size
) {
    int index = 0;
    result_iter_t result = RI_CONTINUE;

    do {
        result = emu_a64_decode_next(emulator, out_buffer, &index, out_buffer_size);
    } while(result == RI_CONTINUE);

    if (result == RI_DONE) {
        return(SUCCESS);
    } else {
        return(FAILURE);
    }
}
