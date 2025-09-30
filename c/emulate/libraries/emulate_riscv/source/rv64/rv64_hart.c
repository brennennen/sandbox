
#include <string.h>

#include "shared/include/result.h"
#include "logger.h"

#include "rv64/rv64_hart.h"
#include "rv64/rv64_shared_system.h"
#include "rv64/rv64_instructions.h"

#include "rv64/rv64_decode_instruction.h"

#include "rv64/instructions/rv64i_base_integer.h"
#include "rv64/instructions/rv64m_multiplication.h"
#include "rv64/instructions/rv64a_atomic.h"
#include "rv64/instructions/rv64f_float.h"
#include "rv64/instructions/rv64v_vector.h"



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

void rv64_hart_set_default_machine_csrs(rv64_hart_t* hart, uint8_t hart_index) {
    rv_mxl_t mxl = RV_MXL_64;
    uint32_t extensions = 0;
    extensions |= RV_EXTENSION_M;
    // Machine Information Registers
    rv64_csr_set_initial_mvendorid(&hart->csrs, 0); // 0 = non-commercial implementation
    rv64_csr_set_initial_marchid(&hart->csrs, 0); // 0 = not implemented (seems to be the best option for an emulator?)
    rv64_csr_set_initial_mimpid(&hart->csrs, 0); // 0 = not implemented
    rv64_csr_set_initial_mhartid(&hart->csrs, hart_index); // 0 = main core (only core)
    rv64_csr_set_initial_mconfigptr(&hart->csrs, 0); // 0 = not implemented (format and schema not standardized yet, so not implementing)
    // Machine Trap Setup
    rv64_csr_set_initial_misa(&hart->csrs, RV_MXL_64, extensions);
    // ...
}

void rv64_hart_set_default_vector_csrs(rv64_hart_t* hart, uint8_t hart_index) {
    rv64_set_csr_value(&hart->csrs, RV64_CSR_VLENB, VLEN / 8);
}

/**
 * Reads 32 bits (4 bytes) from memory starting at the pc register in little-endian.
 */
emu_result_t emu_rv64_read_m32(rv64_hart_t* hart, uint32_t* out_data) {
    if (hart->pc + 1 >= hart->shared_system->memory_size) {
        LOG(LOG_ERROR, "%s: ER_OUT_OF_BOUNDS. ip (+ read size): (%d + 4) >= memory size: %d\n",
            __func__, hart->pc, hart->shared_system->memory_size);
        return(ER_OUT_OF_BOUNDS);
    }
    // TODO: mutex/lock?
    *out_data = (hart->shared_system->memory[hart->pc + 3] << 24)
        | (hart->shared_system->memory[hart->pc + 2] << 16)
        | (hart->shared_system->memory[hart->pc + 1] << 8)
        | (hart->shared_system->memory[hart->pc]);
    if (*out_data != 0) { // if we reached an empty instruction (end of program), don't increment pc.
        hart->pc += 4;
    }
    return(ER_SUCCESS);
}

emu_result_t rv64_hart_init(rv64_hart_t* hart, rv64_shared_system_t* shared_system) {
    hart->shared_system = shared_system;
    // ???
    return(ER_SUCCESS);
}

void debug_print_registers(rv64_hart_t* hart) {
    printf("Registers:\n");
    for (int i = 0; i < 32; i++) {
        if (hart->registers[i] != 0) {
            printf("%s (%d): %lu\n", rv64_map_register_name(i), i, hart->registers[i]);
        }
    }
    printf("PC: %ld\n", hart->pc);
}

static result_iter_t rv64_hart_emulate_next(rv64_hart_t* hart) {
    uint32_t raw_instruction = 0;
    emu_result_t read_result = emu_rv64_read_m32(hart, &raw_instruction);
    LOGD("%s: ip: %d, raw_instruction: %x", __func__, hart->pc - 4, raw_instruction);
    if (hart->instructions_count >= 128) {
        printf("%s: sentinel infinite loop detected, exiting (%d)\n",
            __func__, hart->instructions_count);
        return(RI_DONE);
    }
    // If we reach an empty byte, assume we've hit the end of the program.
    if (raw_instruction == 0x00) {
        return(RI_DONE);
    }

    instruction_tag_rv64_t instruction_tag = I_RV64_INVALID;
    instruction_tag = rv64_decode_instruction_tag(raw_instruction);
    printf("tag: %d\n", instruction_tag);
    hart->instructions_count += 1;

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
            result = rv64i_base_integer_emulate(hart, raw_instruction, instruction_tag);
            break;
        }
        // Zicsr
        case I_RV64ZICSR_CSRRW:
        case I_RV64ZICSR_CSRRS:
        case I_RV64ZICSR_CSRRC:
        case I_RV64ZICSR_CSRRWI:
        case I_RV64ZICSR_CSRRSI:
        case I_RV64ZICSR_CSRRCI: {
            result = rv64i_zicsr_emulate(hart, raw_instruction, instruction_tag);
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
            result = rv64_multiplication_emulate(hart, raw_instruction, instruction_tag);
            break;
        }
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
        case I_RV64ZAAMO_AMOMAXU_D:
        // Zawrs
        case I_RV64ZAWRS_WRS_NTO:
        case I_RV64ZAWRS_WRS_STO:
        // Zacas
        case I_RV64ZACAS_AMOCAS_W:
        case I_RV64ZACAS_AMOCAS_D:
        case I_RV64ZACAS_AMOCAS_Q:
        // Zabha
        case I_RV64ZABHA_AMOSWAP_B:
        case I_RV64ZABHA_AMOADD_B:
        case I_RV64ZABHA_AMOAND_B:
        case I_RV64ZABHA_AMOOR_B:
        case I_RV64ZABHA_AMOXOR_B:
        case I_RV64ZABHA_AMOMAX_B:
        case I_RV64ZABHA_AMOMAXU_B:
        case I_RV64ZABHA_AMOMIN_B:
        case I_RV64ZABHA_AMOMINU_B:
        case I_RV64ZABHA_AMOCAS_B:
        case I_RV64ZABHA_AMOSWAP_H:
        case I_RV64ZABHA_AMOADD_H:
        case I_RV64ZABHA_AMOAND_H:
        case I_RV64ZABHA_AMOOR_H:
        case I_RV64ZABHA_AMOXOR_H:
        case I_RV64ZABHA_AMOMAX_H:
        case I_RV64ZABHA_AMOMAXU_H:
        case I_RV64ZABHA_AMOMIN_H:
        case I_RV64ZABHA_AMOMINU_H:
        case I_RV64ZABHA_AMOCAS_H: {
            result = rv64a_atomic_emulate(hart, raw_instruction, instruction_tag);
            break;
        }
        // TODO: RV64F float/double/quad
        // RV64F
        case I_RV64F_FLW: {
            result = rv64f_float_emulate(hart, raw_instruction, instruction_tag);
            break;
        }
        // ...
        // RV64V
        case I_RV64V_VSETVLI:
        case I_RV64V_VSETVL:
        case I_RV64V_VSETIVLI:
        case I_RV64V_VLE8_V:
        case I_RV64V_VLE16_V:
        case I_RV64V_VLE32_V:
        case I_RV64V_VSE8_V:
        case I_RV64V_VADD_IVV:
        case I_RV64V_VADD_IVX:
        case I_RV64V_VADD_IVI: {
            result = rv64v_vector_emulate(hart, raw_instruction, instruction_tag);
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

result_t run_hart(void* args) {
    rv64_hart_thread_args_t* hart_args = (rv64_hart_thread_args_t*)args;
    rv64_hart_t* hart = hart_args->hart;

    int index = 0;
    result_iter_t result = RI_CONTINUE;

    do {
        result = rv64_hart_emulate_next(hart);
    } while(result == RI_CONTINUE);

    if (result == RI_DONE) {
        return(SUCCESS);
    } else {
        return(FAILURE);
    }
}

/**
 * Copies a a program from a file into memory and executes it.
 */
result_t rv64_hart_emulate_file(
    rv64_hart_t* hart,
    uint64_t memory_address,
    char* input_path
) {
    LOG(LOG_INFO, "Starting emulate file: '%s'", input_path);
    FILE* file = fopen(input_path, "r");
    if (file == NULL) {
        LOG(LOG_ERROR, "Failed to open file: %s\n", input_path);
        return FAILURE;
    }

    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    rewind(file);
    int read_result = fread(hart->shared_system->memory + memory_address, 1, file_size, file);
    if (read_result != file_size) {
        LOG(LOG_ERROR, "Failed to read file!\n");
        return FAILURE;
    }
    // TODO: write 4 null bytes after program.
    hart->pc = memory_address;
    result_t result = rv64_hart_emulate(hart);
    return result;
}

/**
 * Copies a program "chunk" into memory and executes it. Writes 4 extra null bytes
 * past the end of the program to be used as a halt point to detect when the chunk
 * finishes executing.
 */
result_t rv64_hart_emulate_chunk(
    rv64_hart_t* hart,
    uint64_t memory_address,
    char* in_buffer,
    size_t in_buffer_size
) {
    memcpy(hart->shared_system->memory + memory_address, in_buffer, in_buffer_size);
    hart->pc = memory_address;
    // emulator runs until exit is called or a null byte is hit, set some bytes directly
    // after the program to null to facilitate this.
    hart->shared_system->memory[hart->pc + in_buffer_size] = 0x00;
    hart->shared_system->memory[hart->pc + in_buffer_size + 1] = 0x00;
    // todo: memcpy atleast 4 null bytes (1 full instruction)
    //memcpy()
    return(rv64_hart_emulate(hart));
}

result_t rv64_hart_emulate(rv64_hart_t* hart) {
    int index = 0;
    result_iter_t result = RI_CONTINUE;

    do {
        result = rv64_hart_emulate_next(hart);
    } while(result == RI_CONTINUE);

    if (result == RI_DONE) {
        return(SUCCESS);
    } else {
        return(FAILURE);
    }
}

void rv64_print_registers(rv64_hart_t* hart) {
    printf("Registers:\n");
    for (int i = 0; i < 32; i++) {
        if (hart->registers[i] != 0) {
            printf("%s (%d): %lu\n", rv64_map_register_name(i), i, hart->registers[i]);
        }
    }
    printf("PC: %ld\n", hart->pc);
}

void rv64_print_registers_condensed(rv64_hart_t* hart) {
    printf("registers: [pc: %ld] ", hart->pc);
    for (int i = 0; i < 32; i++) {
        if (hart->registers[i] != 0) {
            printf("%s: %lu, ", rv64_map_register_name(i), hart->registers[i]);
        }
    }
    printf("\n");
}
