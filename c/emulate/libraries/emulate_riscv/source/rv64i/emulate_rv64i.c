
#include <string.h>

#include "shared/include/result.h"

#include "logger.h"

#include "rv64i/instructions_rv64i.h"
#include "rv64i/emulate_rv64i.h"
#include "rv64i/emu_rv64i_decode_instruction.h"

#include "rv64i/instructions/arithmetic/add.h"


emu_result_t emu_rv64i_init(emulator_rv64i_t* emulator) {
    emulator->memory_size = MEMORY_SIZE;
    return(ER_SUCCESS);
}

char* emu_rv64i_map_register_name(uint8_t reg_id) {
    // matching objdump -d decode names to make testing easier.
    switch(reg_id) {
        case(0): return("zero"); // always zero
        case(1): return("ra"); // return address
        case(2): return("sp"); // stack pointer
        case(3): return("gp"); // global pointer
        case(4): return("tp"); // thread pointer
        case(5): return("t0"); // temp 0
        case(6): return("t1"); // temp 1
        case(7): return("t2"); // temp 2
        case(8): return("fp"); // TODO: verify this
        case(9): return("s1"); // saved register
        case(10): return("a0"); // argument/return value registers
        case(11): return("a1");
        case(12): return("a2");
        case(13): return("a3");
        case(14): return("a4");
        case(15): return("a5");
        case(16): return("a6");
        case(17): return("a7");
        case(18): return("s2"); // saved registers
        case(19): return("s3");
        case(20): return("s4");
        case(21): return("s5");
        case(22): return("s6");
        case(23): return("s7");
        case(24): return("s8");
        case(25): return("s9");
        case(26): return("s10");
        case(27): return("s11");
        case(28): return("t3"); // temp registers
        case(29): return("t4");
        case(30): return("t5");
        case(31): return("t6");
    }
}

emu_result_t emu_rv64i_read_m32(emulator_rv64i_t* emulator, uint32_t* out_data) {
    if (emulator->registers.pc + 1 >= emulator->memory_size) {
        LOG(LOG_ERROR, "read m32: ER_OUT_OF_BOUNDS. ip (+ read size): (%d + 4) >= memory size: %d\n",
            emulator->registers.pc, emulator->memory_size);
        return(ER_OUT_OF_BOUNDS);
    }
    *out_data = (emulator->memory[emulator->registers.pc] << 24)
        | (emulator->memory[emulator->registers.pc + 1] << 16)
        | (emulator->memory[emulator->registers.pc + 2] << 8)
        | (emulator->memory[emulator->registers.pc + 3]);
    emulator->registers.pc += 4;
    return(ER_SUCCESS);
}

static result_iter_t emu_rv64i_decode_next(
    emulator_rv64i_t* emulator,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
) {
    uint32_t raw_instruction = 0;
    emu_result_t read_result = emu_rv64i_read_m32(emulator, &raw_instruction);
    LOGD("ip: %d, raw_instruction: %x", emulator->registers.pc - 4, raw_instruction);

    // If we reach an empty byte, assume we've hit the end of the program.
    if (raw_instruction == 0x00) {
        return RI_DONE;
    }

    instruction_tag_rv64i_t instruction_tag = I_RV64I_INVALID;
    instruction_tag = emu_rv64i_decode_instruction_tag(raw_instruction);
    emulator->instructions_count += 1;

    emu_result_t result = RI_FAILURE;
    switch(instruction_tag) {
        // ...
        case I_RV64I_ADD_IMMEDIATE: {
            result = emu_rv64i_decode_add_immediate(emulator, raw_instruction, out_buffer, index, out_buffer_size);
            break;
        }
        // ...
    }
    if (result != ER_SUCCESS) {
        fprintf(stderr, "Failed to parse instruction! decode_result = %s (%d)\n", emulate_result_strings[result], result);
        return(RI_FAILURE);
    }

    snprintf(out_buffer + *index, out_buffer_size - *index, "\n");
    *index += 1;

    return(RI_CONTINUE);
}

result_t emu_rv64i_decode_file(
    emulator_rv64i_t* emulator,
    char* input_path,
    char* out_buffer,
    size_t out_buffer_size
) {

    // TODO
    return(ER_FAILURE);
}

result_t emu_rv64i_decode_chunk(
    emulator_rv64i_t* emulator,
    char* in_buffer, size_t in_buffer_size,
    char* out_buffer, size_t out_buffer_size
) {
    memcpy(emulator->memory + PROGRAM_START, in_buffer, in_buffer_size);
    emulator->registers.pc = PROGRAM_START;
    return emu_rv64i_decode(emulator, out_buffer, out_buffer_size);
}

result_t emu_rv64i_decode(
    emulator_rv64i_t* emulator,
    char* out_buffer,
    size_t out_buffer_size
) {
    int index = 0;
    result_iter_t result = RI_CONTINUE;

    do {
        result = emu_rv64i_decode_next(emulator, out_buffer, &index, out_buffer_size);
    } while(result == RI_CONTINUE);

    if (result == RI_DONE) {
        return(SUCCESS);
    } else {
        return(FAILURE);
    }
}
