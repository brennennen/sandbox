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

#include "rv64/rv64_hart.h"
#include "rv64/rv64_shared_system.h"

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


typedef struct emulator_rv64_s {
    uint8_t hart_count;
    rv64_hart_t harts[HART_COUNT];
    rv64_shared_system_t shared_system;
} rv64_emulator_t;

emu_result_t rv64_emulator_init(rv64_emulator_t* emulator);

char* rv64_map_register_name(uint8_t reg_id);
char* rv64_map_vector_register_name(uint8_t vector_reg_id);
char* rv64_map_instruction_tag_mnemonic(instruction_tag_rv64_t tag);

result_t rv64_emulate_file_single_core(rv64_emulator_t* emulator, char* input_path);
result_t rv64_emulate_chunk_single_core(rv64_emulator_t* emulator, char* in_buffer, size_t in_buffer_size);
result_t rv64_emulate(rv64_emulator_t* emulator);

#endif // EMULATE_RV64I_H
