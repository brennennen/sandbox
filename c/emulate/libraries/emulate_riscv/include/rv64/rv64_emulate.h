/**
 * Emulator for RV64I
 */
#ifndef EMULATE_RV64_H
#define EMULATE_RV64_H

#include <stdio.h>
#include <stdint.h>

#include "shared/include/result.h"

#include "rv64/rv64_registers.h"
#include "rv64/rv64_control_status_registers.h"

#include "rv64/rv64_virtual_hardware_conf.h"
#include "rv64/rv64_instructions.h"

// TODO LMUL and SEW

/**
 * @see 30.3.4.2 Vector Register Grouping (`vlmul[2:0]`) (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#vector-register-grouping)
 *
 */
//#define VLMAX ((LMUL*VLEN)/SEW)

// TODO: probably best to make this on the heap
#define MEMORY_SIZE 65535 // 64KB,
#define PROGRAM_START 0x100 // address '0' is usually a forced segfault, write program to some
                            // offset above and leave bytes around 0 for error detection.

typedef union {
    uint8_t bytes[VLEN_BYTES];
    uint8_t elements_8[VLEN_BYTES / 1];
    uint16_t elements_16[VLEN_BYTES / 2];
    uint32_t elements_32[VLEN_BYTES / 4];
    uint64_t elements_64[VLEN_BYTES / 8];
} vector_register_t;


typedef struct emulator_rv64_s {
    uint64_t registers[32];
    uint32_t pc;
    vector_register_t vector_registers[32];
    //registers_rv64_t registers;
    rv64_csrs_t csrs;
    int instructions_count;
    // uint16_t stack_size; // using a size here in case i want to make this dynamic/resizable later.
    // uint16_t stack_top;
    // uint32_t stack[STACK_SIZE];
    uint16_t memory_size;
    uint8_t memory[MEMORY_SIZE];
} emulator_rv64_t;

emu_result_t emu_rv64_init(emulator_rv64_t* emulator);

char* rv64_map_register_name(uint8_t reg_id);
char* rv64_map_vector_register_name(uint8_t vector_reg_id);
char* rv64_map_instruction_tag_mnemonic(instruction_tag_rv64_t tag);

void debug_print_registers(emulator_rv64_t* emulator);

result_t emu_rv64_emulate_file(emulator_rv64_t* emulator, char* input_path);
result_t emu_rv64_emulate_chunk(emulator_rv64_t* emulator, char* in_buffer, size_t in_buffer_size);
result_t emu_rv64_emulate(emulator_rv64_t* emulator);

void emu_rv64_print_registers(emulator_rv64_t* emulator);
void emu_rv64_print_registers_condensed(emulator_rv64_t* emulator);

#endif // EMULATE_RV64I_H
