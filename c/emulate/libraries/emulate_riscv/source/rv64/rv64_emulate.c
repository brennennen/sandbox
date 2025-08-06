
#include <string.h>

#include "shared/include/result.h"

#include "logger.h"

#include "rv64/rv64_instructions.h"
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_decode_instruction.h"

#include "rv64/instructions/rv64i_base_integer.h"
#include "rv64/instructions/rv64m_multiplication.h"



emu_result_t emu_rv64_init(emulator_rv64_t* emulator) {
    emulator->memory_size = MEMORY_SIZE;
    return(ER_SUCCESS);
}

char* rv64_map_register_name(uint8_t reg_id) {
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
        default: return("reg?");
    }
}

char* rv64_map_instruction_tag_mnemonic(instruction_tag_rv64_t tag) {
    return rv64_instruction_tag_mnemonic[tag];
}

/**
 * Reads 32 bits (4 bytes) from memory starting at the pc register in little-endian.
 */
emu_result_t emu_rv64_read_m32(emulator_rv64_t* emulator, uint32_t* out_data) {
    if (emulator->registers.pc + 1 >= emulator->memory_size) {
        LOG(LOG_ERROR, "read m32: ER_OUT_OF_BOUNDS. ip (+ read size): (%d + 4) >= memory size: %d\n",
            emulator->registers.pc, emulator->memory_size);
        return(ER_OUT_OF_BOUNDS);
    }
    *out_data = (emulator->memory[emulator->registers.pc + 3] << 24)
        | (emulator->memory[emulator->registers.pc + 2] << 16)
        | (emulator->memory[emulator->registers.pc + 1] << 8)
        | (emulator->memory[emulator->registers.pc]);
    if (*out_data != 0) { // if we reached an empty instruction (end of program), don't increment pc.
        emulator->registers.pc += 4;
    }
    return(ER_SUCCESS);
}

void debug_print_registers(emulator_rv64_t* emulator) {
    printf("Registers:\n");
    for (int i = 0; i < 32; i++) {
        if (emulator->registers.regs[i] != 0) {
            printf("%s (%d): %lu\n", rv64_map_register_name(i), i, emulator->registers.regs[i]);
        }
    }
    printf("PC: %d\n", emulator->registers.pc);
}

static result_iter_t emu_rv64_emulate_next(emulator_rv64_t* emulator) {
    uint32_t raw_instruction = 0;
    emu_result_t read_result = emu_rv64_read_m32(emulator, &raw_instruction);
    LOGD("ip: %d, raw_instruction: %x", emulator->registers.pc - 4, raw_instruction);
    //emulator->registers.pc += 1;
    // If we reach an empty byte, assume we've hit the end of the program.
    if (raw_instruction == 0x00) {
        return RI_DONE;
    }

    instruction_tag_rv64_t instruction_tag = I_RV64_INVALID;
    instruction_tag = rv64_decode_instruction_tag(raw_instruction);
    printf("tag: %d\n", instruction_tag);
    emulator->instructions_count += 1;

    emu_result_t result = ER_FAILURE;
    switch(instruction_tag) {
        // RV64I
        case I_RV64I_LUI:
        case I_RV64I_AUIPC:
        case I_RV64I_JAL:
        case I_RV64I_BEQ:
        case I_RV64I_BNE:
        case I_RV64I_BLT:
        case I_RV64I_BGE:
        case I_RV64I_BLTU:
        case I_RV64I_BGEU:
        case I_RV64I_JALR:
        case I_RV64I_LB:
        case I_RV64I_LH:
        case I_RV64I_LW:
        case I_RV64I_LBU:
        case I_RV64I_LHU:
        case I_RV64I_SB:
        case I_RV64I_SH:
        case I_RV64I_SW:
        case I_RV64I_ADDI:
        case I_RV64I_SLTI:
        case I_RV64I_SLTIU:
        case I_RV64I_XORI:
        case I_RV64I_ORI:
        case I_RV64I_ANDI:
        case I_RV64I_SLLI:
        case I_RV64I_SRLI:
        case I_RV64I_SRAI:
        case I_RV64I_ADD:
        case I_RV64I_SUB:
        case I_RV64I_SLL:
        case I_RV64I_SLT:
        case I_RV64I_SLTU:
        case I_RV64I_XOR:
        case I_RV64I_SRL:
        case I_RV64I_SRA:
        case I_RV64I_OR:
        case I_RV64I_AND: {
            result = rv64i_base_integer_emulate(emulator, raw_instruction, instruction_tag);
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
            result = rv64_multiplication_emulate(emulator, raw_instruction, instruction_tag);
            break;
        }
        // RV64A
        // ...
        // RV64V
        case I_RV64V_VSETVLI:
        case I_RV64V_VSETVL:
        case I_RV64V_VSETIVLI:{
            printf("todo set vector length/type config operations\n");
            return(RI_FAILURE);
        }
        default: {
            result = ER_FAILURE;
            break;
        }
    }
    if (result != ER_SUCCESS) {
        fprintf(stderr, "Failed to parse instruction! decode_result = %s (%d)\n", emulate_result_strings[result], result);
        return(RI_FAILURE);
    }
    return(RI_CONTINUE);
}

result_t emu_rv64_emulate_file(emulator_rv64_t* emulator, char* input_path) {
    LOG(LOG_INFO, "Starting emulate file: '%s'", input_path);
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
    result_t result = emu_rv64_emulate(emulator);
    return result;
}

result_t emu_rv64_emulate_chunk(emulator_rv64_t* emulator, char* in_buffer, size_t in_buffer_size) {
    memcpy(emulator->memory + PROGRAM_START, in_buffer, in_buffer_size);
    emulator->registers.pc = PROGRAM_START;
    // emulator runs until exit is called or a null byte is hit, set some bytes directly
    // after the program to null to facilitate this.
    emulator->memory[emulator->registers.pc + in_buffer_size] = 0x00;
    emulator->memory[emulator->registers.pc + in_buffer_size + 1] = 0x00;
    return(emu_rv64_emulate(emulator));
}

result_t emu_rv64_emulate(emulator_rv64_t* emulator) {
    int index = 0;
    result_iter_t result = RI_CONTINUE;

    do {
        result = emu_rv64_emulate_next(emulator);
    } while(result == RI_CONTINUE);

    if (result == RI_DONE) {
        return(SUCCESS);
    } else {
        return(FAILURE);
    }
}

void emu_rv64_print_registers(emulator_rv64_t* emulator) {
    printf("registers: [pc: %d] ", emulator->registers.pc);
    for (int i = 0; i < 32; i++) {
        printf("%s: %lu\n", rv64_map_register_name(i), emulator->registers.regs[i]);
    }
}

void emu_rv64_print_registers_condensed(emulator_rv64_t* emulator) {
    printf("registers: [pc: %d] ", emulator->registers.pc);
    for (int i = 0; i < 32; i++) {

        if (emulator->registers.regs[i] != 0) {
            printf("%s: %lu, ", rv64_map_register_name(i), emulator->registers.regs[i]);
        }
    }
    printf("\n");
}
