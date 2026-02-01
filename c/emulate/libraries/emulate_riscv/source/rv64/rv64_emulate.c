#include <string.h>

#include "shared/include/result.h"

#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"

/**
 * RISCV defines which
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/privileged/#reset
 */
static void set_default_csrs(rv64_emulator_t* emulator) {
    for (int i = 0; i < emulator->hart_count; i++) {
        rv64_hart_set_default_machine_csrs(&emulator->harts[i], i);
    }
}

emu_result_t rv64_emulator_init(rv64_emulator_t* emulator) {
    emulator->hart_count = HART_COUNT;
    emulator->shared_system.memory_size = MEMORY_SIZE;

    for (int i = 0; i < HART_COUNT; i++) {
        rv64_hart_init(&emulator->harts[i], &emulator->shared_system);
    }
    set_default_csrs(emulator);
    return ER_SUCCESS;
}

char* rv64_map_instruction_tag_mnemonic(instruction_tag_rv64_t tag) {
    return rv64_instruction_tag_mnemonic[tag];
}

result_t rv64_emulate_file_single_core(rv64_emulator_t* emulator, char* input_path) {
    return rv64_hart_emulate_file(&emulator->harts[0], PROGRAM_START, input_path);
}

result_t rv64_emulate_chunk_single_core(
    rv64_emulator_t* emulator,
    uint8_t* in_buffer,
    size_t in_buffer_size
) {
    return rv64_hart_emulate_chunk(&emulator->harts[0], PROGRAM_START, in_buffer, in_buffer_size);
}
