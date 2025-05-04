/**
 * Emulator for 8086 assembly per the "8086 family users manual 1".
 */
#ifndef EMULATE_H
#define EMULATE_H

#include <stdint.h>
#include <stdio.h>

#include "shared/include/result.h"

#include "8086/emulate_8086.h"

#define STACK_SIZE 4096
#define MEMORY_SIZE 65535 // 64KB (need to add segment register support to address more space)
#define PROGRAM_START 0x100 // address '0' is usually a forced segfault, write program to some
                            // offset above and leave bytes around 0 for error detection.

#define DEBUG 1

typedef enum {
    ARCH_8086, // 8086 (partially implemented)
    ARCH_I386, // i386, x86 (not implemented)
    ARCH_X64, // x64 (not implemented)
} emu_arch_t;

typedef enum {
    BITS_16, // 8086 (partially implemented)
    BITS_32, // i386, x86 (not implemented)
    BITS_64, // x64 (not implemented)
} bits_mode_t;

typedef struct {
    bits_mode_t bits_mode;
    emu_arch_t arch;
    emulator_8086_t emulator_8086;
} emulator_t;




void emu_init(emulator_t* emulator, emu_arch_t arch);



result_t emu_decode_file(emulator_t* emulator, char* input_path, char* out_buffer,
    size_t out_buffer_size);
result_t emu_decode_chunk(emulator_t* emulator, char* in_buffer, size_t in_buffer_size,
    char* out_buffer, size_t out_buffer_size);
result_t emu_decode(emulator_t* emulator, char* out_buffer, size_t out_buffer_size);

result_t emu_emulate_file(emulator_t* emulator, char* input_path);
result_t emu_emulate_chunk(emulator_t* emulator, char* in_buffer, size_t in_buffer_size);
result_t emu_emulate(emulator_t* emulator);

#endif // EMULATE_H
