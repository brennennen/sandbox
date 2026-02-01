/**
 * Emulator for i386
 */
#ifndef EMULATE_i386_H
#define EMULATE_i386_H

#include <stdint.h>
#include <stdio.h>

#include "shared/include/result.h"

#include "i386/registers_i386.h"

#define STACK_SIZE  4096
#define MEMORY_SIZE 65535  // 64KB (need to add segment register support to address more space)
#define PROGRAM_START \
    0x100  // address '0' is usually a forced segfault, write program to some
           // offset above and leave bytes around 0 for error detection.

typedef struct {
    registers_i386_t registers;
    int instructions_count;
    uint16_t stack_size;  // using a size here in case i want to make this dynamic/resizable later.
    uint16_t stack_top;
    uint16_t stack[STACK_SIZE];
    uint16_t memory_size;
    uint8_t memory[MEMORY_SIZE];
} emulator_i386_t;

result_t emu_i386_init(emulator_i386_t* emulator);

#endif
