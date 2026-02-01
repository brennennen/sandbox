
#include <stdint.h>

#include "rv64/disassemble/rv64v_vector_disassemble.h"
#include "rv64/rv64_decode.h"
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"

/**
 * MARK: RV64V
 */

char* rv64v_map_vsew_name(uint8_t vsew) {
    switch (vsew) {
        case (0b000):
            return ("e8");
        case (0b001):
            return ("e16");
        case (0b010):
            return ("e32");
        case (0b011):
            return ("e64");
        default:
            return ("?");
    }
}

char* rv64v_map_vlmul_name(uint8_t vlmul) {
    switch (vlmul) {
        case (0b000):
            return ("m1");
        case (0b001):
            return ("m2");
        case (0b010):
            return ("m4");
        case (0b011):
            return ("m8");
        case (0b100):
            return ("?");
        case (0b101):
            return ("?");
        case (0b110):
            return ("?");
        default:
            return ("?");
    }
}

/**
 * @see 30.6.1 vtype encoding
 * (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_vtype_encoding)
 */
emu_result_t rv64v_disassemble_vsetvli_vtypei(
    rv64_disassembler_t* disassembler,
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

    int written = snprintf(
        buffer + *index, buffer_size - *index, "%s, %s, %s, %s", vsew_name, vlmul_name, vta_name,
        vma_name
    );
    if (written < 0) {
        return (ER_FAILURE);
    }
    *index += written;
    return (ER_SUCCESS);
}

emu_result_t rv64v_disassemble_vsetvli(
    rv64_disassembler_t* disassembler,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    uint8_t rs1 = 0;
    uint8_t rd = 0;
    uint16_t vtypei = 0;
    rv64v_decode_vsetvli(raw_instruction, &rd, &rs1, &vtypei);

    char* rs1_name = rv64_map_register_name(rs1);
    char* rd_name = rv64_map_register_name(rd);
    char* tag_name = rv64_instruction_tag_mnemonic[tag];

    int written = snprintf(
        buffer + *index, buffer_size - *index, "%s %s, %s, ", tag_name, rd_name, rs1_name
    );
    if (written < 0) {
        return (ER_FAILURE);
    }
    *index += written;
    return (rv64v_disassemble_vsetvli_vtypei(disassembler, vtypei, buffer, index, buffer_size));
}

emu_result_t rv64v_disassemble_vsetivli(
    rv64_disassembler_t* disassembler,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    // TODO
    return (ER_FAILURE);
}

emu_result_t rv64v_disassemble_vsetvl(
    rv64_disassembler_t* disassembler,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    // TODO
    return (ER_FAILURE);
}

emu_result_t rv64v_disassemble_opivv(
    rv64_disassembler_t* disassembler,
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
 * 30.5 Vector Instruction Formats
 * LOAD-FP major opcode, VL* unit-stride
 */
emu_result_t rv64v_disassemble_load_unit_stride(
    rv64_disassembler_t* disassembler,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    uint8_t nf = 0;
    uint8_t mew = 0;
    uint8_t mop = 0;
    uint8_t vm = 0;
    uint8_t lumop = 0;
    uint8_t rs1 = 0;
    uint8_t width = 0;
    uint8_t vd = 0;

    rv64v_decode_load_unit_stride(raw_instruction, &nf, &mew, &mop, &vm, &lumop, &rs1, &width, &vd);

    char* tag_name = rv64_instruction_tag_mnemonic[tag];
    char* vd_name = rv64_map_vector_register_name(vd);
    char* rs1_name = rv64_map_register_name(rs1);

    int written = snprintf(
        buffer + *index, buffer_size - *index, "%s %s, (%s)", tag_name, vd_name, rs1_name
    );
    if (written < 0) {
        return (ER_FAILURE);
    }
    *index += written;
    return (ER_SUCCESS);
}

/**
 * 30.5 Vector Instruction Formats
 * STORE-FP major opcode, VL* unit-stride
 */
emu_result_t rv64v_disassemble_store_unit_stride(
    rv64_disassembler_t* disassembler,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    uint8_t nf = 0;
    uint8_t mew = 0;
    uint8_t mop = 0;
    uint8_t vm = 0;
    uint8_t sumop = 0;
    uint8_t rs1 = 0;
    uint8_t width = 0;
    uint8_t vs3 = 0;

    rv64v_decode_load_unit_stride(
        raw_instruction, &nf, &mew, &mop, &vm, &sumop, &rs1, &width, &vs3
    );

    char* tag_name = rv64_instruction_tag_mnemonic[tag];
    char* vs3_name = rv64_map_vector_register_name(vs3);
    char* rs1_name = rv64_map_register_name(rs1);

    int written = snprintf(
        buffer + *index, buffer_size - *index, "%s %s, (%s)", tag_name, vs3_name, rs1_name
    );
    if (written < 0) {
        return (ER_FAILURE);
    }
    *index += written;
    return (ER_SUCCESS);
}

emu_result_t rv64v_integer_vector_vector_disassemble(
    rv64_disassembler_t* disassembler,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    uint8_t vm = 0;
    uint8_t vs2 = 0;
    uint8_t vs1 = 0;
    uint8_t vd = 0;
    rv64v_decode_vector_vector(raw_instruction, &vm, &vs2, &vs1, &vd);

    char* tag_name = rv64_instruction_tag_mnemonic[tag];
    char* vs2_name = rv64_map_vector_register_name(vs2);
    char* vs1_name = rv64_map_vector_register_name(vs1);
    char* vd_name = rv64_map_vector_register_name(vd);

    int written = snprintf(
        buffer + *index, buffer_size - *index, "%s %s, %s, %s", tag_name, vd_name, vs2_name,
        vs1_name
    );
    if (written < 0) {
        return (ER_FAILURE);
    }
    *index += written;
    return (ER_SUCCESS);
}

emu_result_t rv64v_integer_vector_scalar_disassemble(
    rv64_disassembler_t* disassembler,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    uint8_t vm = 0;
    uint8_t vs2 = 0;
    uint8_t rs1 = 0;
    uint8_t vd = 0;
    rv64v_decode_vector_scalar(raw_instruction, &vm, &vs2, &rs1, &vd);

    char* tag_name = rv64_instruction_tag_mnemonic[tag];
    char* vs2_name = rv64_map_vector_register_name(vs2);
    char* rs1_name = rv64_map_register_name(rs1);
    char* vd_name = rv64_map_vector_register_name(vd);

    int written = snprintf(
        buffer + *index, buffer_size - *index, "%s %s, %s, %s", tag_name, vd_name, vs2_name,
        rs1_name
    );
    if (written < 0) {
        return (ER_FAILURE);
    }
    *index += written;
    return (ER_SUCCESS);
}

emu_result_t rv64v_integer_vector_immediate_disassemble(
    rv64_disassembler_t* disassembler,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    uint8_t vm = 0;
    uint8_t vs2 = 0;
    uint8_t imm = 0;
    uint8_t vd = 0;
    rv64v_decode_vector_immediate(raw_instruction, &vm, &vs2, &imm, &vd);

    char* tag_name = rv64_instruction_tag_mnemonic[tag];
    char* vs2_name = rv64_map_vector_register_name(vs2);
    char* vd_name = rv64_map_vector_register_name(vd);

    int written = snprintf(
        buffer + *index, buffer_size - *index, "%s %s, %s, %d", tag_name, vd_name, vs2_name, imm
    );
    if (written < 0) {
        return (ER_FAILURE);
    }
    *index += written;
    return (ER_SUCCESS);
}

/*
 * MARK: RV64V Main
 */
emu_result_t rv64v_vector_disassemble(
    rv64_disassembler_t* disassembler,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
) {
    emu_result_t result = ER_FAILURE;
    // RV64V
    switch (tag) {
        // todo: vector admin/setup
        case I_RV64V_VSETVLI: {
            result = rv64v_disassemble_vsetvli(
                disassembler, raw_instruction, tag, buffer, index, buffer_size
            );
            break;
        }
        case I_RV64V_VSETIVLI: {
            result = rv64v_disassemble_vsetivli(
                disassembler, raw_instruction, tag, buffer, index, buffer_size
            );
            break;
        }
        case I_RV64V_VSETVL: {
            result = rv64v_disassemble_vsetvl(
                disassembler, raw_instruction, tag, buffer, index, buffer_size
            );
            break;
        }
        // load
        case I_RV64V_VLE8_V:
        case I_RV64V_VLE16_V:
        case I_RV64V_VLE32_V:
        case I_RV64V_VLE64_V: {
            result = rv64v_disassemble_load_unit_stride(
                disassembler, raw_instruction, tag, buffer, index, buffer_size
            );
            break;
        }
        // store
        case I_RV64V_VSE8_V:
        case I_RV64V_VSE16_V:
        case I_RV64V_VSE32_V:
        case I_RV64V_VSE64_V: {
            result = rv64v_disassemble_store_unit_stride(
                disassembler, raw_instruction, tag, buffer, index, buffer_size
            );
            break;
        }
        // opivv - integer vector-vector
        case I_RV64V_VADD_IVV:
        case I_RV64V_VSUB_IVV: {
            result = rv64v_integer_vector_vector_disassemble(
                disassembler, raw_instruction, tag, buffer, index, buffer_size
            );
            break;
        }
        // opivx - integer vector-scalar
        case I_RV64V_VADD_IVX:
        case I_RV64V_VSUB_IVX:
        case I_RV64V_VRSUB_IVX: {
            result = rv64v_integer_vector_scalar_disassemble(
                disassembler, raw_instruction, tag, buffer, index, buffer_size
            );
            break;
        }
        // opivi - integer vector-immediate
        case I_RV64V_VADD_IVI:
        case I_RV64V_VRSUB_IVI: {
            result = rv64v_integer_vector_immediate_disassemble(
                disassembler, raw_instruction, tag, buffer, index, buffer_size
            );
            break;
        }

        // ...
        default: {
            printf("rv64v_vector_disassemble: not implemented\n");
            return (ER_FAILURE);
        }
    }

    // uint8_t aquire = 0;
    // uint8_t release = 0;
    // uint8_t rs2 = 0;
    // uint8_t rs1 = 0;
    // uint8_t rd = 0;
    // rv64a_decode_atomic(raw_instruction, &aquire, &release, &rs2, &rs1, &rd);

    // printf("rv64a_atomic_disassemble: aquire: %d, release: %d, rs2: %d, rs1: %d, rd: %d\n",
    //     aquire, release, rs2, rs1, rd);

    // char* rs1_name = rv64_map_register_name(rs1);
    // char* rs2_name = rv64_map_register_name(rs2);
    // char* rd_name = rv64_map_register_name(rd);
    // char* tag_name = rv64_instruction_tag_mnemonic[tag];

    // int written = snprintf(buffer + *index, buffer_size - *index,
    //     "%s %s, %s, (%s)", tag_name, rd_name, rs2_name, rs1_name);

    // if (written < 0) {
    //     return(ER_FAILURE);
    // }
    // *index += written;
    return (ER_SUCCESS);
}
