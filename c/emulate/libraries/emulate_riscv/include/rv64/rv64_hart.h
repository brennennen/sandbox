#ifndef RV64_HART_H
#define RV64_HART_H

#include <stdint.h>

#include "rv64/rv64_control_status_registers.h"
#include "rv64/rv64_virtual_hardware_conf.h"
#include "rv64/rv64_shared_system.h"

typedef union {
    uint8_t bytes[VLEN_BYTES];
    uint8_t elements_8[VLEN_BYTES / 1];
    uint16_t elements_16[VLEN_BYTES / 2];
    uint32_t elements_32[VLEN_BYTES / 4];
    uint64_t elements_64[VLEN_BYTES / 8];
} vector_register_t;

typedef struct rv64_hart_s {
    uint64_t pc;
    uint64_t registers[32];
    float float32_registers[32]; // "F" module
    double float64_registers[32]; // "D" module
    // todo: "Q" module 128 bit float
    vector_register_t vector_registers[32]; // "V" module
    rv64_csrs_t csrs;
    int instructions_count;
    rv64_shared_system_t* shared_system;
    // uint16_t stack_size; // using a size here in case i want to make this dynamic/resizable later.
    // uint16_t stack_top;
    // uint32_t stack[STACK_SIZE];
} rv64_hart_t;

typedef struct hart_thread_args_s {
    rv64_hart_t* hart;
    // ???
} rv64_hart_thread_args_t;

emu_result_t rv64_hart_init(rv64_hart_t* hart, rv64_shared_system_t* shared_system);

void rv64_hart_set_default_machine_csrs(rv64_hart_t* hart, uint8_t hart_index);
void debug_print_registers(rv64_hart_t* hart);

result_t rv64_hart_emulate_file(rv64_hart_t* hart, uint64_t memory_address, char* input_path);
result_t rv64_hart_emulate_chunk(rv64_hart_t* hart, uint64_t memory_address, char* in_buffer, size_t in_buffer_size);
result_t rv64_hart_emulate(rv64_hart_t* hart);

void rv64_print_registers(rv64_hart_t* hart);
void rv64_print_registers_condensed(rv64_hart_t* hart);


#endif // RV64_HART_H
