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

#include "libraries/emulate_intel/include/emulate.h"
#include "libraries/emulate_intel/include/emu_registers.h"
#include "libraries/emulate_intel/include/decode_utils.h"
#include "libraries/emulate_intel/include/decode_tag.h"
#include "libraries/emulate_intel/include/logger.h"

#include "libraries/emulate_intel/include/8086/emulate_8086.h"
#include "libraries/emulate_intel/include/8086/decode_8086.h"

#include "libraries/emulate_intel/include/i386/decode_i386.h"

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

//#include "decode_tag.c"

/**
 * Initializes the decoder.
 */
void emu_init(emulator_t* emulator) {
    emulator->stack_size = STACK_SIZE; // using a size here in case i want to make this dynamic/resizable later.
    emulator->stack_top = 0;
    emulator->memory_size = MEMORY_SIZE;
    emulator->bits_mode = BITS_16;
    memset(emulator->stack, 0, emulator->stack_size);
}

result_t emu_memory_set_byte(emulator_t* emulator, uint32_t address, uint8_t value) {
    if (address < emulator->memory_size) {
        emulator->memory[address] = value;
        return ER_SUCCESS;
    }
    return ER_FAILURE;
}

result_t emu_memory_set_uint16(emulator_t* emulator, uint32_t address, uint16_t value) {
    if (address + 1 < emulator->memory_size) {
        memcpy(&emulator->memory[address], &value, 2);
        return ER_SUCCESS;
    }
    return ER_FAILURE;
}

result_t emu_memory_get_byte(emulator_t* emulator, uint32_t address, uint8_t* out_value) {
    if (address < emulator->memory_size) {
        *out_value = emulator->memory[address];
        return ER_SUCCESS;
    }
    return ER_FAILURE;
}

result_t emu_memory_get_uint16(emulator_t* emulator, uint32_t address, uint16_t* out_value) {
    if (address + 1 < emulator->memory_size) {
        memcpy((uint8_t*)out_value, &emulator->memory[address], 2);
        return ER_SUCCESS;
    }
    return ER_FAILURE;
}

result_t emu_decode_file(
    emulator_t* emulator,
    char* input_path,
    char* out_buffer,
    size_t out_buffer_size)
{
    LOG(LOG_INFO, "Starting decode file: '%s'", input_path);
    FILE* file = fopen(input_path, "r");
    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    rewind(file);
    int read_result = fread(emulator->memory + PROGRAM_START, 1, file_size, file);
    if (read_result != file_size) {
        LOG(LOG_ERROR, "Failed to read file!\n");
        return FAILURE;
    }
    emulator->registers.ip = PROGRAM_START;
    result_t result = emu_decode(emulator, out_buffer, out_buffer_size);
    return result;
}

result_t emu_decode_chunk(
    emulator_t* emulator,
    char* in_buffer,
    size_t in_buffer_size,
    char* out_buffer,
    size_t out_buffer_size)
{
    memcpy(emulator->memory + PROGRAM_START, in_buffer, in_buffer_size);
    emulator->registers.ip = PROGRAM_START;
    return emu_decode(emulator, out_buffer, out_buffer_size);
}

result_t emu_decode(emulator_t* emulator, char* out_buffer, size_t out_buffer_size) {
    switch(emulator->bits_mode) {
        case BITS_16: {
            return(emu_8086_decode(emulator, out_buffer, out_buffer_size));
        }
        case BITS_32: {
            return(emu_i386_decode(emulator, out_buffer, out_buffer_size));
        }
        default: {
            LOGD("i386 and x64 (32 and 64 bit modes) are not implemented");
            return(FAILURE);
        }
    }
}

result_t emu_emulate_file(emulator_t* emulator, char* input_path) {
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
    result_t result = emu_emulate(emulator);
    return result;
}

result_t emu_emulate_chunk(
    emulator_t* emulator,
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

    return emu_emulate(emulator);
}

result_t emu_emulate(emulator_t* emulator) {
    switch(emulator->bits_mode) {
        case BITS_16: {
            return(emu_8086_emulate(emulator));
        }
        default: {
            LOGD("i386 and x64 (32 and 64 bit modes) are not implemented");
            return(FAILURE);
        }
    }
}
