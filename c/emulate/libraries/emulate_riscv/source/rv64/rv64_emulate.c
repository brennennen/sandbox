
#include <string.h>

#include "shared/include/result.h"

#include "logger.h"

#include "rv64/rv64_instructions.h"
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_decode_instruction.h"

#include "rv64/instructions/rv64i_base_integer.h"
#include "rv64/instructions/rv64m_multiplication.h"
#include "rv64/instructions/rv64a_atomic.h"
#include "rv64/instructions/rv64v_vector.h"

/**
 * RISCV defines which
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/privileged/#reset
 */
static void set_default_machine_csrs(emulator_rv64_t* emulator) {
    rv_mxl_t mxl = RV_MXL_64;
    uint32_t extensions = 0;
    extensions |= RV_EXTENSION_M;
    // Machine Information Registers
    rv64_csr_set_initial_mvendorid(&emulator->csrs, 0); // 0 = non-commercial implementation
    rv64_csr_set_initial_marchid(&emulator->csrs, 0); // 0 = not implemented (seems to be the best option for an emulator?)
    rv64_csr_set_initial_mimpid(&emulator->csrs, 0); // 0 = not implemented
    rv64_csr_set_initial_mhartid(&emulator->csrs, 0); // 0 = main core (only core)
    rv64_csr_set_initial_mconfigptr(&emulator->csrs, 0); // 0 = not implemented (format and schema not standardized yet, so not implementing)
    // Machine Trap Setup
    rv64_csr_set_initial_misa(&emulator->csrs, RV_MXL_64, extensions);
    // ...
}

emu_result_t emu_rv64_init(emulator_rv64_t* emulator) {
    emulator->memory_size = MEMORY_SIZE;
    set_default_machine_csrs(emulator);
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
        default: return("r??");
    }
}

char* rv64_map_vector_register_name(uint8_t vector_reg_id) {
    switch(vector_reg_id) {
        case(0): return("v0");
        case(1): return("v1");
        case(2): return("v2");
        case(3): return("v3");
        case(4): return("v4");
        case(5): return("v5");
        case(6): return("v6");
        case(7): return("v7");
        case(8): return("v8");
        case(9): return("v9");
        case(10): return("v10");
        case(11): return("v11");
        case(12): return("v12");
        case(13): return("v13");
        case(14): return("v14");
        case(15): return("v15");
        case(16): return("v16");
        case(17): return("v17");
        case(18): return("v18");
        case(19): return("v19");
        case(20): return("v20");
        case(21): return("v21");
        case(22): return("v22");
        case(23): return("v23");
        case(24): return("v24");
        case(25): return("v25");
        case(26): return("v26");
        case(27): return("v27");
        case(28): return("v28");
        case(29): return("v29");
        case(30): return("v30");
        case(31): return("v31");
        default: return("v??");
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
    if (emulator->instructions_count >= 128) {
        printf("%s:sentinel infinite loop detected, exiting (%d)\n",
            __func__, emulator->instructions_count);
        return(RI_DONE);
    }
    // If we reach an empty byte, assume we've hit the end of the program.
    if (raw_instruction == 0x00) {
        return(RI_DONE);
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
        case I_RV64I_AND:
        case I_RV64I_FENCE:
        case I_RV64I_FENCE_TSO:
        case I_RV64I_PAUSE:
        case I_RV64I_ECALL:
        case I_RV64I_EBREAK:
        case I_RV64I_LWU:
        case I_RV64I_LD:
        case I_RV64I_SD:
        case I_RV64I_ADDIW:
        case I_RV64I_SLLIW:
        case I_RV64I_SRLIW:
        case I_RV64I_SRAIW:
        case I_RV64I_ADDW:
        case I_RV64I_SUBW:
        case I_RV64I_SLLW:
        case I_RV64I_SRLW:
        case I_RV64I_SRAW: {
            result = rv64i_base_integer_emulate(emulator, raw_instruction, instruction_tag);
            break;
        }
        // Zicsr
        case I_RV64ZICSR_CSRRW:
        case I_RV64ZICSR_CSRRS:
        case I_RV64ZICSR_CSRRC:
        case I_RV64ZICSR_CSRRWI:
        case I_RV64ZICSR_CSRRSI:
        case I_RV64ZICSR_CSRRCI: {
            result = rv64i_zicsr_emulate(emulator, raw_instruction, instruction_tag);
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
        // TODO: RV64F float/double/quad
        // RV64A
        case I_RV64ZALRSC_LR_W:
        case I_RV64ZALRSC_SC_W:
        case I_RV64ZAAMO_AMOSWAP_W:
        case I_RV64ZAAMO_AMOADD_W:
        case I_RV64ZAAMO_AMOXOR_W:
        case I_RV64ZAAMO_AMOAND_W:
        case I_RV64ZAAMO_AMOOR_W:
        case I_RV64ZAAMO_AMOMIN_W:
        case I_RV64ZAAMO_AMOMAX_W:
        case I_RV64ZAAMO_AMOMINU_W:
        case I_RV64ZAAMO_AMOMAXU_W:
        case I_RV64ZALRSC_LR_D:
        case I_RV64ZALRSC_SC_D:
        case I_RV64ZAAMO_AMOSWAP_D:
        case I_RV64ZAAMO_AMOADD_D:
        case I_RV64ZAAMO_AMOXOR_D:
        case I_RV64ZAAMO_AMOAND_D:
        case I_RV64ZAAMO_AMOOR_D:
        case I_RV64ZAAMO_AMOMIN_D:
        case I_RV64ZAAMO_AMOMAX_D:
        case I_RV64ZAAMO_AMOMINU_D:
        case I_RV64ZAAMO_AMOMAXU_D: {
            result = rv64a_atomic_emulate(emulator, raw_instruction, instruction_tag);
            break;
        }
        // ...
        // RV64V
        case I_RV64V_VSETVLI:
        case I_RV64V_VSETVL:
        case I_RV64V_VSETIVLI:
        case I_RV64V_VLE8_V:
        case I_RV64V_VLE16_V:
        case I_RV64V_VSE8_V: {
            result = rv64v_vector_emulate(emulator, raw_instruction, instruction_tag);
            break;
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
