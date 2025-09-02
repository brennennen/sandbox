#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "logger.h"

#include "rv64/rv64_decode.h"
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"

#include "rv64/instructions/rv64v_vector.h"


/**
 * @see 30.6 Configuration-Setting Instructions (vsetvli/vsetivli/vsetvl) (https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#sec-vector-config)
 *
 * vsetvli rd, rs1, vtypei # rd = new vl, rs1 = AVL, vtypei = new vtype setting
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
    printf("%s\n", __func__);

    uint64_t old_vtype = emulator->csrs.vtype;
    // TODO: decode vtypei?
    // uint8_t vma = (vtypei >> 7) & 0b1;
    // uint8_t vta = (vtypei >> 6) & 0b1;
    // uint8_t vsew = (vtypei >> 3) & 0b111;
    // uint8_t vlmul = vtypei & 0b111;

    emulator->csrs.vtype = vtypei;
    // todo: should "vsetvli" be setting vl?

    if (rd != 0) {
        emulator->registers.regs[rd] = old_vtype;
    }
}

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

    rv64v_vtype_t vtype = rv64_csr_decode_vtype(emulator->csrs.vtype);
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
        case I_RV64V_VLE8_V: {
            rv64v_vle8_v(emulator, raw_instruction, tag);
            break;
        }

        // todo: the rest of the owl
        default: {
            return(ER_FAILURE);
        }
    }
    return(ER_SUCCESS);
}
