


#include "i386/instructions/logic/and.h"

#include "i386/decode_i386_common.h"

#include "logger.h"

emu_result_t emu_i386_decode_and(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
) {
    return(emu_i386_decode_and_write_common_standard_format(emulator, byte1,
        "and", 3, out_buffer, index, out_buffer_size));
}
