
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

emu_result_t rv64_disassemble_upper_immediate(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    int32_t imm20 = 0;
    uint8_t rd = 0;

    rv64_decode_upper_immediate(raw_instruction, &imm20, &rd);

    char* rd_name = rv64_map_register_name(rd);
    char* tag_name = rv64_instruction_tag_mnemonic[tag];

    int written = snprintf(buffer + *index, buffer_size - *index,
        "%s %s, %d", tag_name, rd_name, imm20);
    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
    return(ER_SUCCESS);
}

/**
 * Disassemble "J-Type" instruction format (I think jal is the only one?).
 * Uses the gnu relative address notation (. + <offset>).
 * @see 2.5.1 Unconditional Jumps (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_unconditional_jumps)
 */
emu_result_t rv64_disassemble_jal(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    int32_t offset = 0;
    uint8_t rd = 0;

    rv64_decode_j_type(raw_instruction, &offset, &rd);

    // TODO: pseudoinstruction "j" when rd is x0
    char* rd_name = rv64_map_register_name(rd);
    char* tag_name = rv64_instruction_tag_mnemonic[tag];


    int written = snprintf(buffer + *index, buffer_size - *index,
        "%s %s, . + %d", tag_name, rd_name, offset);
    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
    return(ER_SUCCESS);
}

emu_result_t rv64_disassemble_branch(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    int32_t offset = 0;
    uint8_t rs1 = 0;
    uint8_t rs2 = 0;

    rv64_decode_branch(raw_instruction, &offset, &rs1, &rs2);

    char* rs1_name = rv64_map_register_name(rs1);
    char* rs2_name = rv64_map_register_name(rs2);
    char* tag_name = rv64_instruction_tag_mnemonic[tag];

    int written = snprintf(buffer + *index, buffer_size - *index,
        "%s %s, %s, . + %d", tag_name, rs1_name, rs2_name, offset);
    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
    return(ER_SUCCESS);
}

emu_result_t rv64_disassemble_load(
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

    char* rs1_name = rv64_map_register_name(rs1);
    char* rd_name = rv64_map_register_name(rd);
    char* tag_name = rv64_instruction_tag_mnemonic[tag];

    int written = snprintf(buffer + *index, buffer_size - *index,
        "%s %s, %d(%s)", tag_name, rd_name, imm12, rs1_name);
    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
    return(ER_SUCCESS);
}

emu_result_t rv64_disassemble_store(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    uint16_t offset = 0;
    uint8_t rs1 = 0;
    uint8_t rs2 = 0;

    rv64_decode_store(raw_instruction, &offset, &rs1, &rs2);

    char* rs1_name = rv64_map_register_name(rs1);
    char* rs2_name = rv64_map_register_name(rs2);
    char* tag_name = rv64_instruction_tag_mnemonic[tag];

    int written = snprintf(buffer + *index, buffer_size - *index,
        "%s %s, %d(%s)", tag_name, rs2_name, offset, rs1_name);
    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
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

    char* rs1_name = rv64_map_register_name(rs1);
    char* rd_name = rv64_map_register_name(rd);
    char* tag_name = rv64_instruction_tag_mnemonic[tag];

    int written = snprintf(buffer + *index, buffer_size - *index,
        "%s %s, %s, %d", tag_name, rd_name, rs1_name, imm12);
    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
    return(ER_SUCCESS);
}

emu_result_t rv64_disassemble_shift_immediate(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    uint8_t imm5 = 0;
    uint8_t rs1 = 0;
    uint8_t rd = 0;

    rv64_decode_shift_immediate(raw_instruction, &imm5, &rs1, &rd);

    char* rs1_name = rv64_map_register_name(rs1);
    char* rd_name = rv64_map_register_name(rd);
    char* tag_name = rv64_instruction_tag_mnemonic[tag];

    int written = snprintf(buffer + *index, buffer_size - *index,
        "%s %s, %s, %d", tag_name, rd_name, rs1_name, imm5);
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

    char* rs1_name = rv64_map_register_name(rs1);
    char* rs2_name = rv64_map_register_name(rs2);
    char* rd_name = rv64_map_register_name(rd);
    char* tag_name = rv64_instruction_tag_mnemonic[tag];

    int written = snprintf(buffer + *index, buffer_size - *index,
        "%s %s, %s, %s", tag_name, rd_name, rs1_name, rs2_name);

    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
    return(ER_SUCCESS);
}

emu_result_t rv64_disassemble_no_args(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    char* tag_name = rv64_instruction_tag_mnemonic[tag];
    int written = snprintf(buffer + *index, buffer_size - *index,
        "%s", tag_name);

    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
    return(ER_SUCCESS);
}

emu_result_t rv64_disassemble_csr_register(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    uint8_t csr = 0;
    uint8_t rs1 = 0;
    uint8_t rd = 0;

    rv64_decode_csr_register(raw_instruction, &csr, &rs1, &rd);

    char* csr_name = rv64_csr_name_from_address(csr);
    char* rs1_name = rv64_map_register_name(rs1);
    char* rd_name = rv64_map_register_name(rd);
    char* tag_name = rv64_instruction_tag_mnemonic[tag];

    int written = snprintf(buffer + *index, buffer_size - *index,
        "%s %s, %s, %s", tag_name, rd_name, csr_name, rs1_name);

    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
    return(ER_SUCCESS);
}

emu_result_t rv64_disassemble_csr_immediate(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    uint8_t csr = 0;
    uint8_t uimm = 0;
    uint8_t rd = 0;

    rv64_decode_csr_immediate(raw_instruction, &csr, &uimm, &rd);

    char* csr_name = rv64_csr_name_from_address(csr);
    char* rd_name = rv64_map_register_name(rd);
    char* tag_name = rv64_instruction_tag_mnemonic[tag];

    int written = snprintf(buffer + *index, buffer_size - *index,
        "%s %s, %s, %d", tag_name, rd_name, csr_name, uimm);

    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
    return(ER_SUCCESS);
}

/**
 * MARK: RV64V
 */

char* rv64v_map_vsew_name(uint8_t vsew) {
    switch(vsew) {
        case(0b000): return("e8");
        case(0b001): return("e16");
        case(0b010): return("e32");
        case(0b011): return("e64");
        default: return("?");
    }
}

char* rv64v_map_vlmul_name(uint8_t vlmul) {
    switch(vlmul) {
        case(0b000): return("m1");
        case(0b001): return("m2");
        case(0b010): return("m4");
        case(0b011): return("m8");
        case(0b100): return("?");
        case(0b101): return("?");
        case(0b110): return("?");
        default: return("?");
    }
}

/**
 * @see 30.6.1 vtype encoding (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_vtype_encoding)
 */
emu_result_t rv64v_disassemble_vsetvli_vtypei(
    emulator_rv64_t* emulator,
    uint16_t vtypei,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    uint8_t vill = 0;
    uint8_t vma = 0;
    uint8_t vta = 0;
    uint8_t vsew = 0;
    uint8_t vlmul = 0;
    rv64v_decode_vsetvli_vtypei(vtypei, &vill, &vma, &vta, &vsew, &vlmul);

    char* vsew_name = rv64v_map_vsew_name(vsew);
    char* vlmul_name = rv64v_map_vlmul_name(vlmul);
    char* vma_name = "ma";
    if (vma == 0) {
        vma_name = "mu";
    }
    char* vta_name = "ta";
    if (vta == 0) {
        vta_name = "tu";
    }

    int written = snprintf(buffer + *index, buffer_size - *index,
        "%s, %s, %s, %s", vsew_name, vlmul_name, vta_name, vma_name);
    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
    return(ER_SUCCESS);
}

emu_result_t rv64v_disassemble_vsetvli(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    uint8_t rs1 = 0;
    uint8_t rd = 0;
    uint16_t vtypei = 0;
    rv64v_decode_vsetvli(raw_instruction, &rs1, &rd, &vtypei);

    char* rs1_name = rv64_map_register_name(rs1);
    char* rd_name = rv64_map_register_name(rd);
    char* tag_name = rv64_instruction_tag_mnemonic[tag];

    int written = snprintf(buffer + *index, buffer_size - *index,
        "%s %s, %s, ", tag_name, rs1_name, rd_name);
    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
    return(rv64v_disassemble_vsetvli_vtypei(emulator, vtypei, buffer, index, buffer_size));
}

emu_result_t rv64v_disassemble_vsetivli(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    // TODO
    return(ER_FAILURE);
}

emu_result_t rv64v_disassemble_vsetvl(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    // TODO
    return(ER_FAILURE);
}

emu_result_t rv64v_disassemble_opivv(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    uint8_t vs1 = 0;
    uint8_t vs2 = 0;
    uint8_t vd = 0;
    uint8_t vm = 0;

    rv64v_decode_opivv(raw_instruction, &vs1, &vs2, &vd, &vm);
    // todo
}

/**
 * MARK: MAIN
 */

// TODO: move to shared func
emu_result_t emu_rv64_disassemble_read_m32(emulator_rv64_t* emulator, uint32_t* out_data) {
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
        // Core Format "U" - Upper Immediate
        case I_RV64I_LUI:
        case I_RV64I_AUIPC: {
            result = rv64_disassemble_upper_immediate(emulator, raw_instruction, instruction_tag, out_buffer, index, out_buffer_size);
            break;
        }
        // Core Format "J" - Jump?
        case I_RV64I_JAL: {
            result = rv64_disassemble_jal(emulator, raw_instruction, instruction_tag, out_buffer, index, out_buffer_size);
            break;
        }
        // "I" format
        case I_RV64I_JALR: {
            result = rv64_disassemble_register_immediate(emulator, raw_instruction, instruction_tag, out_buffer, index, out_buffer_size);
            break;
        }

        // Core Format "B" - Branch
        case I_RV64I_BEQ:
        case I_RV64I_BNE:
        case I_RV64I_BLT:
        case I_RV64I_BGE:
        case I_RV64I_BLTU:
        case I_RV64I_BGEU: {
            result = rv64_disassemble_branch(emulator, raw_instruction, instruction_tag, out_buffer, index, out_buffer_size);
            break;
        }

        // Load ("I" format)
        case I_RV64I_LB:
        case I_RV64I_LH:
        case I_RV64I_LW:
        case I_RV64I_LBU:
        case I_RV64I_LHU:
        case I_RV64I_LWU:
        case I_RV64I_LD: {
            result = rv64_disassemble_load(emulator, raw_instruction, instruction_tag, out_buffer, index, out_buffer_size);
            break;
        }

        // Store ("S" format)
        case I_RV64I_SB:
        case I_RV64I_SH:
        case I_RV64I_SW:
        case I_RV64I_SD: {
            result = rv64_disassemble_store(emulator, raw_instruction, instruction_tag, out_buffer, index, out_buffer_size);
            break;
        }

        // Core Format "I" - "register-immediate"
        case I_RV64I_ADDI:
        case I_RV64I_SLTI:
        case I_RV64I_SLTIU:
        case I_RV64I_XORI:
        case I_RV64I_ORI:
        case I_RV64I_ANDI:
        case I_RV64I_ADDIW: {
            result = rv64_disassemble_register_immediate(emulator, raw_instruction, instruction_tag, out_buffer, index, out_buffer_size);
            break;
        }
        // Special format "shift-immediate"
        case I_RV64I_SLLI:
        case I_RV64I_SRLI:
        case I_RV64I_SRAI:
        case I_RV64I_SLLIW:
        case I_RV64I_SRLIW:
        case I_RV64I_SRAIW: {
            result = rv64_disassemble_shift_immediate(emulator, raw_instruction, instruction_tag, out_buffer, index, out_buffer_size);
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
        case I_RV64I_SRA:
        case I_RV64I_OR:
        case I_RV64I_AND:
        case I_RV64I_ADDW:
        case I_RV64I_SUBW:
        case I_RV64I_SLLW:
        case I_RV64I_SRLW:
        case I_RV64I_SRAW: {
            result = rv64_disassemble_register_register(emulator, raw_instruction, instruction_tag, out_buffer, index, out_buffer_size);
            break;
        }

        // instructions that don't have arguments
        case I_RV64I_FENCE:
        case I_RV64I_FENCE_TSO:
        case I_RV64I_PAUSE:
        case I_RV64I_ECALL:
        case I_RV64I_EBREAK: {
            result = rv64_disassemble_no_args(emulator, raw_instruction, instruction_tag, out_buffer, index, out_buffer_size);
            break;
        }

        // RV64Zifenceei
        case I_RV64ZIFENCEI_FENCE_I: {
            result = rv64_disassemble_no_args(emulator, raw_instruction, instruction_tag, out_buffer, index, out_buffer_size);
            break;
        }
        // RV64Zicsr
        case I_RV64ZICSR_CSRRW:
        case I_RV64ZICSR_CSRRS:
        case I_RV64ZICSR_CSRRC: {
            result = rv64_disassemble_csr_register(emulator, raw_instruction, instruction_tag, out_buffer, index, out_buffer_size);
            break;
        }
        case I_RV64ZICSR_CSRRWI:
        case I_RV64ZICSR_CSRRSI:
        case I_RV64ZICSR_CSRRCI: {
            result = rv64_disassemble_csr_immediate(emulator, raw_instruction, instruction_tag, out_buffer, index, out_buffer_size);
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
        case I_RV64F_FLW: {
            // todo: separate file?
        }
        // RV64D
        // RV64Q
        // RV64V
        // todo: vector admin/setup
        case I_RV64V_VSETVLI: {
            result = rv64v_disassemble_vsetvli(emulator, raw_instruction, instruction_tag, out_buffer, index, out_buffer_size);
            break;
        }
        case I_RV64V_VSETIVLI: {
            result = rv64v_disassemble_vsetivli(emulator, raw_instruction, instruction_tag, out_buffer, index, out_buffer_size);
            break;
        }
        case I_RV64V_VSETVL: {
            result = rv64v_disassemble_vsetvl(emulator, raw_instruction, instruction_tag, out_buffer, index, out_buffer_size);
            break;
        }
        // todo: vector load and store
        // todo: vector arithmetic
        case I_RV64V_VADD_VV:
        case I_RV64V_VADD_VX:
        case I_RV64V_VADD_VI:
        case I_RV64V_VSUB_VV:
        case I_RV64V_VSUB_VX:
        case I_RV64V_VRSUB_VX:
        case I_RV64V_VRSUB_VI: {
            //result = rv64v_disassemble_vector(emulator, raw_instruction, instruction_tag, out_buffer, index, out_buffer_size);
        }
        default: {
            printf("emu_rv64_disassemble_next instruction not supported! %d\n", raw_instruction);
            result = RI_FAILURE;
            break;
        }
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
