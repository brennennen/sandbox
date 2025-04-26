/**
 * Decodes machine code for 8086 assembly per the "8086 family users manual 1".
 */
#ifndef DECODE_i386_H
#define DECODE_i386_H

#include <stdio.h>

#include "shared/include/result.h"
#include "i386/emulate_i386.h"

result_t emu_i386_decode_chunk(
    emulator_i386_t* emulator,
    char* in_buffer,
    size_t in_buffer_size,
    char* out_buffer,
    size_t out_buffer_size);
result_t emu_i386_decode(emulator_i386_t* emulator, char* out_buffer, size_t out_buffer_size);

#endif // DECODE_i386_H
