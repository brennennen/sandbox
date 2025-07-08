
#include <string.h>

#include "shared/include/result.h"

#include "logger.h"

#include "rv64/rv64_instructions.h"
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_decode_instruction.h"
#include "rv64/rv64_decode.h"
#include "rv64/rv64_disassemble.h"

emu_result_t rv64_disassemble_init(emulator_rv64_t* emulator) {
    emulator->memory_size = MEMORY_SIZE;
    return(ER_SUCCESS);
}

emu_result_t rv64_disassemble_register_immediate(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    int16_t imm12 = 0;
    uint8_t rs1 = 0;
    uint8_t rd = 0;

    rv64_decode_register_immediate(raw_instruction, &imm12, &rs1, &rd);

    char* rs1_name = emu_rv64_map_register_name(rs1);
    char* rd_name = emu_rv64_map_register_name(rd);
    char* tag_name = rv64_instruction_tag_mnemonic[tag];

    int written = snprintf(buffer + *index, buffer_size - *index,
        "%s %s, %s, %d", tag_name, rd_name, rs1_name, imm12);
    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
    return(ER_SUCCESS);
}

emu_result_t rv64_disassemble_register_register(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    uint8_t rs2 = 0;
    uint8_t rs1 = 0;
    uint8_t rd = 0;

    rv64_decode_register_register(raw_instruction, &rs2, &rs1, &rd);

    char* rs1_name = emu_rv64_map_register_name(rs1);
    char* rs2_name = emu_rv64_map_register_name(rs2);
    char* rd_name = emu_rv64_map_register_name(rd);
    char* tag_name = rv64_instruction_tag_mnemonic[tag];

    int written = snprintf(buffer + *index, buffer_size - *index,
        "%s %s, %s, %s", tag_name, rd_name, rs1_name, rs2_name);

    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
    return(ER_SUCCESS);
}

// TODO: move to shared func
emu_result_t emu_rv64_disassemble_read_m32(emulator_rv64_t* emulator, uint32_t* out_data) {
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

static result_iter_t emu_rv64_disassemble_next(
    emulator_rv64_t* emulator,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
) {
    uint32_t raw_instruction = 0;
    emu_result_t read_result = emu_rv64_disassemble_read_m32(emulator, &raw_instruction);
    LOGD("ip: %d, raw_instruction: %x", emulator->registers.pc - 4, raw_instruction);

    // If we reach an empty byte, assume we've hit the end of the program.
    if (raw_instruction == 0x00) {
        return RI_DONE;
    }

    instruction_tag_rv64_t instruction_tag = I_RV64_INVALID;
    instruction_tag = rv64_decode_instruction_tag(raw_instruction);
    emulator->instructions_count += 1;

    emu_result_t result = RI_FAILURE;
    switch(instruction_tag) {
        // RV64I
        // Core Format "I" - "register-immediate"
        case I_RV64I_JALR:
        case I_RV64I_LB:
        case I_RV64I_LH:
        case I_RV64I_LW:
        case I_RV64I_LBU:
        case I_RV64I_LHU:
        case I_RV64I_ADDI:
        case I_RV64I_SLTI:
        case I_RV64I_SLTIU:
        case I_RV64I_XORI:
        case I_RV64I_ORI:
        case I_RV64I_ANDI: {
            result = rv64_disassemble_register_immediate(emulator, raw_instruction, instruction_tag, out_buffer, index, out_buffer_size);
            break;
        }
        // Core Format "R" - "register-register"
        case I_RV64I_ADD:
        case I_RV64I_SUB:
        case I_RV64I_SLL:
        case I_RV64I_SLT:
        case I_RV64I_SLTU:
        case I_RV64I_XOR:
        case I_RV64I_SRL:
        case I_RV64I_OR:
        case I_RV64I_AND: {
            result = rv64_disassemble_register_register(emulator, raw_instruction, instruction_tag, out_buffer, index, out_buffer_size);
            break;
        }
        // RV64M
        case I_RV64M_MUL:
        case I_RV64M_MULH:
        case I_RV64M_MULHSU:
        case I_RV64M_MULHU:
        case I_RV64M_DIV:
        case I_RV64M_DIVU:
        case I_RV64M_REM:
        case I_RV64M_REMU:
        case I_RV64M_MULW:
        case I_RV64M_DIVW:
        case I_RV64M_DIVUW:
        case I_RV64M_REMW:
        case I_RV64M_REMUW: {
            result = rv64_disassemble_register_register(emulator, raw_instruction, instruction_tag, out_buffer, index, out_buffer_size);
            break;
        }
        // RV64A
        // RV64F
        // RV64D
        // RV64Q
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

result_t emu_rv64_disassemble_file(
    emulator_rv64_t* emulator,
    char* input_path,
    char* out_buffer,
    size_t out_buffer_size
) {
    LOG(LOG_INFO, "Starting disassemble file: '%s'", input_path);
    FILE* file = fopen(input_path, "r");
    if (file == NULL) {
        LOG(LOG_ERROR, "Failed to open file: %s\n", input_path);
        return FAILURE;
    }

    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    rewind(file);
    int read_result = fread(emulator->memory + PROGRAM_START, 1, file_size, file);
    if (read_result != file_size) {
        LOG(LOG_ERROR, "Failed to read file!\n");
        return FAILURE;
    }
    emulator->registers.pc = PROGRAM_START;
    result_t result = emu_rv64_disassemble(emulator, out_buffer, out_buffer_size);
    return result;
}

result_t emu_rv64_disassemble_chunk(
    emulator_rv64_t* emulator,
    char* in_buffer, size_t in_buffer_size,
    char* out_buffer, size_t out_buffer_size
) {
    memcpy(emulator->memory + PROGRAM_START, in_buffer, in_buffer_size);
    emulator->registers.pc = PROGRAM_START;
    return(emu_rv64_disassemble(emulator, out_buffer, out_buffer_size));
}

result_t emu_rv64_disassemble(
    emulator_rv64_t* emulator,
    char* out_buffer,
    size_t out_buffer_size
) {
    int index = 0;
    result_iter_t result = RI_CONTINUE;

    do {
        result = emu_rv64_disassemble_next(emulator, out_buffer, &index, out_buffer_size);
    } while(result == RI_CONTINUE);

    if (result == RI_DONE) {
        return(SUCCESS);
    } else {
        return(FAILURE);
    }
}
