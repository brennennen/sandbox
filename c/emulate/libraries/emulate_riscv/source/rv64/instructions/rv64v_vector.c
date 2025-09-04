#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "shared/include/math_utilities.h"


#include "logger.h"

#include "rv64/rv64_decode.h"
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"

#include "rv64/instructions/rv64v_vector.h"


/**
 * Calculates the Maximum Vector Length.
 *
 * `VLMAX = LMUL * VLEN / SEW`
 *
 * See Unprivileged spec table near the bottom of 30.3.4.2 "Vector Register Grouping"
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#vector-register-grouping
 */
static uint64_t rv64v_calculate_vlmax(rv64v_vtype_t* vtype) {
    // start simple, vlmul == 0, is group size of 1.
    if (vtype->vlmul == 0) {
        // todo: avoid integer division, use a look up table?
        return (uint64_t)(VLEN / (uint64_t)vtype->selected_element_width);
    }
    // TODO: group sizes not 1
    printf("setting vlmax to 0, 'group sizes != 1' is not supported!\n");
    return 0;
}

/**
 * Configures vector csrs (vl and vtype) for any upcoming vector instruction operations.
 *
 * @see 30.6 Configuration-Setting Instructions (vsetvli/vsetivli/vsetvl) (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#sec-vector-config)
 *
 * `vsetvli rd, rs1, vtypei` # rd = new vl, rs1 = AVL, vtypei = new vtype setting
 * The `vtypei` is broken down into it's 4 fields
 * `vsetvli rd, rs1, sew, lmul, ta, ma`
 * ex:
 * `vsetvli rd, rs1, e8, m8, ta, ma`
 */
static void rv64v_vsetvli(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    uint8_t rs1 = 0;
    uint8_t rd = 0;
    uint16_t vtypei = 0;
    rv64v_decode_vsetvli(raw_instruction, &rs1, &rd, &vtypei);

    // Set Vector Type
    // todo: detect illegal vtype
    // todo: going from vtypei (10 bits) to vtype (64 bits) do we need to do something while widening?
    emulator->csrs.vtype = vtypei;

    // Set "Vector Length"
    rv64v_vtype_t vtype;
    rv64_csr_decode_vtype(emulator->csrs.vtype, &vtype);
    uint64_t avl = emulator->registers.regs[rs1];
    uint64_t vlmax = rv64v_calculate_vlmax(&vtype);
    emulator->csrs.vl = MIN(avl, vlmax);

    if (rd != 0) {
        emulator->registers.regs[rd] = emulator->csrs.vl;
    }
}

/**
 *
 */
static void rv64v_vle8_v(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
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

    printf("%s\n", __func__);

    if (emulator->csrs.vl == 0) {
        // trigger illegal trap?
        printf("%s: csrs.vl == 0! vector instruction has nothing to do!\n", __func__);
        return;
    }

    const uint64_t base_addr = emulator->registers.regs[rs1];
    printf("%s:base_addr: %ld, mem: %d\n", __func__, base_addr, emulator->memory[base_addr]);
    printf("%s:mem: %d, %d, %d, ...\n", __func__,
        emulator->memory[base_addr], emulator->memory[base_addr + 1], emulator->memory[base_addr + 2]);

    rv64v_vtype_t vtype;
    rv64_csr_decode_vtype(emulator->csrs.vtype, &vtype);
    if (vtype.selected_element_width != RV64_SEW_8) {
        // trigger illegal trap?
        printf("%s: sew != SEW_8! vector instruction undefined throw illegal trap!\n", __func__);
        return;
    }

    // TODO: masking
    uint8_t vlen_bytes = 16; // vlen = 128 bits
    for (uint64_t i = 0; i < emulator->csrs.vl; i++) {
        uint64_t physical_vd = vd + (i / vlen_bytes);
        uint64_t byte_offset = i % vlen_bytes;
        emulator->registers.vregs[physical_vd][byte_offset] = emulator->memory[base_addr + i];
        printf("%s:phys_vd: %ld, byte_offset: %ld, addr: %ld, mem: %d\n",
            __func__, physical_vd, byte_offset, base_addr + i, emulator->memory[base_addr + i]);
    }
}

static void rv64v_vle16_v(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
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

    printf("%s\n", __func__);

    if (emulator->csrs.vl == 0) {
        // trigger illegal trap?
        printf("%s: csrs.vl == 0! vector instruction has nothing to do!\n", __func__);
        return;
    }

    const uint64_t base_addr = emulator->registers.regs[rs1];
    printf("%s:base_addr: %ld, mem: %d\n", __func__, base_addr, emulator->memory[base_addr]);
    printf("%s:mem: %d, %d, %d, ...\n", __func__,
        emulator->memory[base_addr], emulator->memory[base_addr + 1], emulator->memory[base_addr + 2]);

    rv64v_vtype_t vtype;
    rv64_csr_decode_vtype(emulator->csrs.vtype, &vtype);
    if (vtype.selected_element_width != RV64_SEW_16) {
        // trigger illegal trap?
        printf("%s: sew != RV64_SEW_16! vector instruction undefined throw illegal trap!\n", __func__);
        return;
    }

    // TODO: masking

    uint8_t vlen_bytes = 8; // vlen = 128 bits
    for (uint64_t i = 0; i < emulator->csrs.vl; i++) {
        uint64_t physical_vd = vd + (i / vlen_bytes);
        uint64_t byte_offset = i % vlen_bytes;
        emulator->registers.vregs[physical_vd][byte_offset] = emulator->memory[base_addr + i];
        printf("%s:phys_vd: %ld, byte_offset: %ld, addr: %ld, mem: %d\n",
            __func__, physical_vd, byte_offset, base_addr + i, emulator->memory[base_addr + i]);
    }
}

static void rv64v_vse8_v(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
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

    printf("%s\n", __func__);

    if (emulator->csrs.vl == 0) {
        // trigger illegal trap?
        printf("%s: csrs.vl == 0! vector instruction has nothing to do!\n", __func__);
        return;
    }

    const uint64_t base_addr = emulator->registers.regs[rs1];
    printf("%s:base_addr: %ld, mem: %d\n", __func__, base_addr, emulator->memory[base_addr]);
    printf("%s:mem: %d, %d, %d, ...\n", __func__,
        emulator->memory[base_addr], emulator->memory[base_addr + 1], emulator->memory[base_addr + 2]);

    rv64v_vtype_t vtype;
    rv64_csr_decode_vtype(emulator->csrs.vtype, &vtype);
    if (vtype.selected_element_width != RV64_SEW_8) {
        // trigger illegal trap?
        printf("%s: sew != SEW_8! vector instruction undefined throw illegal trap!\n", __func__);
        return;
    }

    // TODO: masking
    uint8_t vlen_bytes = 16; // vlen = 128 bits
    for (uint64_t i = 0; i < emulator->csrs.vl; i++) {
        uint64_t physical_vd = vd + (i / vlen_bytes);
        uint64_t byte_offset = i % vlen_bytes;
        emulator->memory[base_addr + i] = emulator->registers.vregs[physical_vd][byte_offset];
        printf("%s:phys_vd: %ld, byte_offset: %ld, addr: %ld, mem: %d\n",
            __func__, physical_vd, byte_offset, base_addr + i, emulator->memory[base_addr + i]);
    }
}


emu_result_t rv64v_vector_emulate(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    printf("%s\n", __func__);
    // RV64V
    switch (tag) {
        // vector admin/config
        case I_RV64V_VSETVLI: {
            rv64v_vsetvli(emulator, raw_instruction, tag);
            break;
        }
        // case I_RV64V_VSETIVLI: {
        //     rv64v_emulate_vsetivli(emulator, raw_instruction, tag);
        //     break;
        // }
        // case I_RV64V_VSETVL: {
        //     rv64v_emulate_vsetvl(emulator, raw_instruction, tag);
        //     break;
        // }
        // load
        case I_RV64V_VLE8_V: {
            rv64v_vle8_v(emulator, raw_instruction, tag);
            break;
        }
        case I_RV64V_VLE16_V: {
            rv64v_vle16_v(emulator, raw_instruction, tag);
            break;
        }
        // ...
        // store
        case I_RV64V_VSE8_V: {
            rv64v_vse8_v(emulator, raw_instruction, tag);
            break;
        }
        // ...
        // arithmetic

        // todo: the rest of the owl
        default: {
            return(ER_FAILURE);
        }
    }
    return(ER_SUCCESS);
}
