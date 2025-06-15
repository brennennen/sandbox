/**
 *
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/result.h"

#include "emulate.h"
#include "logger.h"

#include "8086/emulate_8086.h"
#include "i386/decode_i386.h"

/**
 * Initializes the decoder.
 */
void emu_init(emulator_t* emulator,arch_t arch) {
    switch(arch) {
        case ARCH_I8086: {
            emu_8086_init(&emulator->emulator_8086);
        }
        default: {
            LOGD("ARCH not implemented");
        }
    }
}

result_t emu_disassemble_file(
    emulator_t* emulator,
    char* input_path,
    char* out_buffer,
    size_t out_buffer_size)
{
    switch(emulator->arch) {
        case ARCH_I8086: {
            return(emu_8086_disassemble_file(&emulator->emulator_8086, input_path, out_buffer, out_buffer_size));
        }
        default: {
            LOGD("ARCH not implemented");
            return(FAILURE);
        }
    }
}

result_t emu_disassemble_chunk(
    emulator_t* emulator,
    char* in_buffer,
    size_t in_buffer_size,
    char* out_buffer,
    size_t out_buffer_size)
{
    switch(emulator->arch) {
        case ARCH_I8086: {
            return(emu_8086_disassemble_chunk(&emulator->emulator_8086, in_buffer, in_buffer_size, out_buffer, out_buffer_size));
        }
        default: {
            LOGD("ARCH not implemented");
            return(FAILURE);
        }
    }
}

result_t emu_disassemble(emulator_t* emulator, char* out_buffer, size_t out_buffer_size) {
    switch(emulator->arch) {
        case ARCH_I8086: {
            return(emu_8086_disassemble(&emulator->emulator_8086, out_buffer, out_buffer_size));
        }
        default: {
            LOGD("ARCH not implemented");
            return(FAILURE);
        }
    }
}

result_t emu_emulate_file(emulator_t* emulator, char* input_path) {
    LOG(LOG_INFO, "Starting emulate file: %s\n", input_path);
    switch(emulator->arch) {
        case ARCH_I8086: {
            return(emu_8086_emulate_file(&emulator->emulator_8086, input_path));
        }
        default: {
            LOGD("ARCH not implemented");
            return(FAILURE);
        }
    }
}

result_t emu_emulate_chunk(
    emulator_t* emulator,
    char* in_buffer,
    size_t in_buffer_size
) {
    switch(emulator->arch) {
        case ARCH_I8086: {
            return(emu_8086_emulate_chunk(&emulator->emulator_8086, in_buffer, in_buffer_size));
        }
        default: {
            LOGD("ARCH not implemented");
            return(FAILURE);
        }
    }
}

result_t emu_emulate(emulator_t* emulator) {
    switch(emulator->bits_mode) {
        case BITS_16: {
            // TODO:
            return(emu_8086_emulate(&emulator->emulator_8086));
        }
        default: {
            LOGD("ARCH not implemented");
            return(FAILURE);
        }
    }
}
