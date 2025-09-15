#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "shared/include/math_utilities.h"


#include "logger.h"

#include "rv64/rv64_decode.h"
#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"

#include "rv64/instructions/rv64v_vector.h"

/*
 * MARK: Helpers
 */

static uint8_t rv64v_selected_element_width(rv64v_selected_element_width_t sew) {
    switch(sew) {
        case RV64_SEW_8: return 8;
        case RV64_SEW_16: return 16;
        case RV64_SEW_32: return 32;
        case RV64_SEW_64: return 64;
        default: {
            printf("%s: invalid sew enum: %d. defaulting to 8.\n", __func__, sew);
            return 8;
        }
    }
}

/**
 * Calculates the Maximum Vector Length.
 *
 * `VLMAX = LMUL * VLEN / SEW`
 *
 * See Unprivileged spec table near the bottom of 30.3.4.2 "Vector Register Grouping"
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#vector-register-grouping
 */
static uint64_t rv64v_calculate_vlmax(rv64v_vtype_t* vtype) {
    // todo: avoid integer division, use a look up table?
    // if (vtype->selected_element_width == 0) {
    //     printf("%s: \n", __func__);
    //     return(0);
    // }
    uint8_t sew = rv64v_selected_element_width(vtype->selected_element_width);
    uint64_t base_vlmax = (uint64_t)(VLEN / (uint64_t)sew);
    uint64_t vlmax = 0;
    switch(vtype->vlmul) {
        case RV64_VLMUL_1: {
            vlmax = base_vlmax;
            break;
        }
        case RV64_VLMUL_2: {
            vlmax = base_vlmax * 2;
            break;
        }
        case RV64_VLMUL_4: {
            vlmax = base_vlmax * 4;
            break;
        }
        case RV64_VLMUL_8: {
            vlmax = base_vlmax * 8;
            break;
        }
        // TODO: fractional vlmuls
        default: {
            printf("%s: setting vlmax to 0, fractional vlmuls not supported yet\n", __func__);
            vlmax = 0;
        }
    }
    printf("%s: vlmul: %d, vlmax: %ld\n", __func__, vtype->vlmul, vlmax);
    return vlmax;

    // start simple, vlmul == 0, is group size of 1.
    // if (vtype->vlmul == RV64_VLMUL_1) {
    //     // todo: avoid integer division, use a look up table?
    //     return (uint64_t)(VLEN / (uint64_t)vtype->selected_element_width);
    // }
    // // TODO: group sizes not 1
    // printf("setting vlmax to 0, 'group sizes != 1' is not supported!\n");
    // return 0;
}

static bool rv64v_is_register_group_aligned(
    uint8_t vector_register,
    rv64v_vlmul_t vlmul
) {
    bool is_aligned = true;
    switch(vlmul) {
        case RV64_VLMUL_2: {
            if (vector_register % 2 != 0) {
                is_aligned = false;
            }
            break;
        }
        case RV64_VLMUL_4: {
            if (vector_register % 4 != 0) {
                is_aligned = false;
            }
            break;
        }
        case RV64_VLMUL_8: {
            if (vector_register % 8 != 0) {
                is_aligned = false;
            }
            break;
        }
        // TODO: fractional vlmuls?
        default: {
            printf("%s: unsupported vlmul\n", __func__);
            is_aligned = false;
            break;
        }
    }
    return is_aligned;
}

/*
 * MARK: Configure vtype/vl
 */

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
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    uint8_t rs1 = 0;
    uint8_t rd = 0;
    uint16_t vtypei = 0;
    rv64v_decode_vsetvli(raw_instruction, &rd, &rs1, &vtypei);

    // Set Vector Type
    // todo: detect illegal vtype
    // todo: going from vtypei (10 bits) to vtype (64 bits) do we need to do something while widening?
    hart->csrs.vtype = vtypei;

    // Set "Vector Length"
    rv64v_vtype_t vtype;
    rv64_csr_decode_vtype(hart->csrs.vtype, &vtype);
    uint64_t avl = hart->registers[rs1];
    uint64_t vlmax = rv64v_calculate_vlmax(&vtype);

    hart->csrs.vl = MIN(avl, vlmax);
    printf("%s: avl reg/rs1: %d\n", __func__, rs1);
    printf("%s: avl: %ld, vlmax: %ld, vl: %ld\n", __func__, avl, vlmax, hart->csrs.vl);

    if (rd != 0) {
        hart->registers[rd] = hart->csrs.vl;
    }
}

/*
 * MARK: Loads
 */
/*
// Minimal implementation of a vector load without vector register grouping or masking.
static void rv64v_vle8_v_simple_no_grouping_no_masking(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    uint8_t nf, mew, mop, vm, lumop, rs1, width, vd = 0;
    rv64v_decode_load_unit_stride(raw_instruction, &nf, &mew, &mop, &vm, &lumop, &rs1, &width, &vd);
    const uint64_t base_addr = hart->registers[rs1];
    uint8_t vlen_bytes = 16; // vlen = 128 bits
    for (uint64_t i = 0; i < hart->csrs.vl; i++) {
        uint64_t physical_vd = vd + (i / vlen_bytes);
        uint64_t byte_offset = i % vlen_bytes;
        hart->vector_registers[physical_vd].bytes[byte_offset] = hart->memory[base_addr + i];
    }
}
*/




/**
 * `vle8.v` - Vector Load Element (8 bits each)
 * `vle8.v vs1, (rs2)`
 * Loads 8 bit elements from memory into vector registers with "unit striding"
 * (no gaps/offsets between elements).
 */
static void rv64v_vle8_v(
    rv64_hart_t* hart,
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

    if (hart->csrs.vl == 0) {
        // trigger illegal trap?
        printf("%s: csrs.vl == 0! vector instruction has nothing to do!\n", __func__);
        return;
    }

    const uint64_t base_addr = hart->registers[rs1];
    rv64v_vtype_t vtype;
    rv64_csr_decode_vtype(hart->csrs.vtype, &vtype);
    if (!rv64v_is_register_group_aligned(vd, vtype.vlmul)) {
        printf("%s: vegister group not aligned: %d, vlmul: %d\n", __func__, vd, vtype.vlmul);
    }

    uint8_t vlen_bytes = 16; // vlen = 128 bits
    for (uint64_t i = 0; i < hart->csrs.vl; i++) {
        bool mask_elem_active = true;
        if (vm == 0) {
            // todo: set mask_elem_active accordingly
        } else {
            mask_elem_active = true; // masking disabled, so always load everything
        }
        if (mask_elem_active) {
            uint64_t physical_vd = vd + (i / vlen_bytes);
            uint64_t byte_offset = i % vlen_bytes;
            hart->vector_registers[physical_vd].bytes[byte_offset] = hart->shared_system->memory[base_addr + i];
        }
    }
}

/**
 * `vle16.v` - Vector Load Element (16 bits each)
 * `vle16.v vs1, (rs2)`
 * Loads 16 bit elements from memory into vector registers with "unit striding"
 * (no gaps/offsets between elements).
 */
static void rv64v_vle16_v(
    rv64_hart_t* hart,
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

    if (hart->csrs.vl == 0) {
        // todo: trigger illegal trap?
        printf("%s: csrs.vl == 0! vector instruction has nothing to do!\n", __func__);
        return;
    }

    const uint64_t base_addr = hart->registers[rs1];
    rv64v_vtype_t vtype;
    rv64_csr_decode_vtype(hart->csrs.vtype, &vtype);
    if (!rv64v_is_register_group_aligned(vd, vtype.vlmul)) {
        printf("%s: vegister group not aligned: %d, vlmul: %d\n", __func__, vd, vtype.vlmul);
    }

    uint8_t elements_per_vector = VLEN / 16; // assuming vlen=128: 128 / 16 = 8 elems per vector register
    for (int i = 0; i < hart->csrs.vl; i++) {
        bool mask_elem_active = true;
        if (vm == 0) {
            // todo: set mask_elem_active accordingly
        } else {
            mask_elem_active = true; // masking disabled, so always load everything
        }
        if (mask_elem_active) {
            uint8_t physical_vd = vd + (i / elements_per_vector);
            uint16_t elem_offset = i % elements_per_vector;
            uint64_t elem_address = base_addr + (uint64_t)(i * 2);
            memcpy(&hart->vector_registers[physical_vd].elements_16[elem_offset],
                &hart->shared_system->memory[elem_address], 2);
        }

    }
}

/**
 * `vle32.v` - Vector Load Element (32 bits each)
 * `vle32.v vs1, (rs2)`
 * Loads 32 bit elements from memory into vector registers with "unit striding"
 * (no gaps/offsets between elements).
 */
static void rv64v_vle32_v(
    rv64_hart_t* hart,
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

    if (hart->csrs.vl == 0) {
        // todo: trigger illegal trap?
        printf("%s: csrs.vl == 0! vector instruction has nothing to do!\n", __func__);
        return;
    }

    const uint64_t base_addr = hart->registers[rs1];
    rv64v_vtype_t vtype;
    rv64_csr_decode_vtype(hart->csrs.vtype, &vtype);
    if (!rv64v_is_register_group_aligned(vd, vtype.vlmul)) {
        printf("%s: vegister group not aligned: %d, vlmul: %d\n", __func__, vd, vtype.vlmul);
    }

    uint8_t elements_per_vector = VLEN / 32; // assuming vlen=128: 128 / 32 = 4 elems per vector register
    for (int i = 0; i < hart->csrs.vl; i++) {
        bool mask_elem_active = true;
        if (vm == 0) {
            // todo: set mask_elem_active accordingly
        } else {
            mask_elem_active = true; // masking disabled, so always load everything
        }
        if (mask_elem_active) {
            uint8_t physical_vd = vd + (i / elements_per_vector);
            uint16_t elem_offset = i % elements_per_vector;
            uint64_t elem_address = base_addr + (uint64_t)(i * 4);
            memcpy(&hart->vector_registers[physical_vd].elements_32[elem_offset],
                &hart->shared_system->memory[elem_address], 4);
        }
    }
}


// rv64v_vlse8_V



/*
 * MARK: Stores
 */

static void rv64v_vse8_v(
    rv64_hart_t* hart,
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

    if (hart->csrs.vl == 0) {
        // todo: trigger illegal trap?
        printf("%s: csrs.vl == 0! vector instruction has nothing to do!\n", __func__);
        return;
    }

    const uint64_t base_addr = hart->registers[rs1];

    rv64v_vtype_t vtype;
    rv64_csr_decode_vtype(hart->csrs.vtype, &vtype);
    if (vtype.selected_element_width != RV64_SEW_8) {
        // todo: trigger illegal trap?
        printf("%s: sew != SEW_8! vector instruction undefined throw illegal trap!\n", __func__);
        return;
    }

    // TODO: masking
    uint8_t vlen_bytes = 16; // vlen = 128 bits
    for (uint64_t i = 0; i < hart->csrs.vl; i++) {
        uint64_t physical_vd = vd + (i / vlen_bytes);
        uint64_t byte_offset = i % vlen_bytes;
        hart->shared_system->memory[base_addr + i] = hart->vector_registers[physical_vd].bytes[byte_offset];
    }
}

/*
 * MARK: Arithmetic
 */

/**
 * Vector-vector addition
 * 30.11.1. Vector Single-Width Integer Add and Subtract
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_vector_single_width_integer_add_and_subtract
 */
static void rv64v_vadd_vv(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    uint8_t vm = 0;
    uint8_t vs2_idx = 0;
    uint8_t vs1_idx = 0;
    uint8_t vd_idx = 0;
    rv64v_decode_vector_vector(raw_instruction, &vm, &vs2_idx, &vs1_idx, &vd_idx);

    if (hart->csrs.vl == 0) {
        // todo: trigger illegal trap?
        printf("%s: csrs.vl == 0! vector instruction has nothing to do!\n", __func__);
        return;
    }

    rv64v_vtype_t vtype;
    rv64_csr_decode_vtype(hart->csrs.vtype, &vtype);

    vector_register_t* vs1 = &hart->vector_registers[vs1_idx];
    vector_register_t* vs2 = &hart->vector_registers[vs2_idx];
    vector_register_t* vd = &hart->vector_registers[vd_idx];

    for (int i = 0; i < hart->csrs.vl; i++) {
        bool mask_elem_active = true;
        if (vm == 0) {
            // todo: set mask_elem_active accordingly
        } else {
            mask_elem_active = true; // masking disabled, so always load everything
        }
        if (mask_elem_active) {
            switch(vtype.selected_element_width) {
                case RV64_SEW_8: {
                    vd->elements_8[i] = vs2->elements_8[i] + vs1->elements_8[i];
                    break;
                }
                case RV64_SEW_16: {
                    vd->elements_16[i] = vs2->elements_16[i] + vs1->elements_16[i];
                    break;
                }
                case RV64_SEW_32: {
                    vd->elements_32[i] = vs2->elements_32[i] + vs1->elements_32[i];
                    break;
                }
                case RV64_SEW_64: {
                    vd->elements_64[i] = vs2->elements_64[i] + vs1->elements_64[i];
                    break;
                }
            }
        }
    }
}

/**
 * Vector-scalar addition
 * 30.11.1. Vector Single-Width Integer Add and Subtract
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_vector_single_width_integer_add_and_subtract
 */
static void rv64v_vadd_vx(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    uint8_t vm = 0;
    uint8_t vs2_idx = 0;
    uint8_t rs1_idx = 0;
    uint8_t vd_idx = 0;
    rv64v_decode_vector_scalar(raw_instruction, &vm, &vs2_idx, &rs1_idx, &vd_idx);

    if (hart->csrs.vl == 0) {
        // todo: trigger illegal trap?
        printf("%s: csrs.vl == 0! vector instruction has nothing to do!\n", __func__);
        return;
    }

    rv64v_vtype_t vtype;
    rv64_csr_decode_vtype(hart->csrs.vtype, &vtype);

    int64_t rs1 = hart->registers[rs1_idx];
    vector_register_t* vs2 = &hart->vector_registers[vs2_idx];
    vector_register_t* vd = &hart->vector_registers[vd_idx];

    for (int i = 0; i < hart->csrs.vl; i++) {
        bool mask_elem_active = true;
        if (vm == 0) {
            // todo: set mask_elem_active accordingly
        } else {
            mask_elem_active = true; // masking disabled, so always load everything
        }
        if (mask_elem_active) {
            switch(vtype.selected_element_width) {
                case RV64_SEW_8: {
                    vd->elements_8[i] = vs2->elements_8[i] + rs1;
                    break;
                }
                case RV64_SEW_16: {
                    vd->elements_16[i] = vs2->elements_16[i] + rs1;
                    break;
                }
                case RV64_SEW_32: {
                    vd->elements_32[i] = vs2->elements_32[i] + rs1;
                    break;
                }
                case RV64_SEW_64: {
                    vd->elements_64[i] = vs2->elements_64[i] + rs1;
                    break;
                }
            }
        }
    }
}

/**
 * Vector immediate addition
 * 30.11.1. Vector Single-Width Integer Add and Subtract
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_vector_single_width_integer_add_and_subtract
 */
static void rv64v_vadd_vi(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    uint8_t vm = 0;
    uint8_t vs2_idx = 0;
    uint8_t imm = 0;
    uint8_t vd_idx = 0;
    rv64v_decode_vector_immediate(raw_instruction, &vm, &vs2_idx, &imm, &vd_idx);

    if (hart->csrs.vl == 0) {
        // todo: trigger illegal trap?
        printf("%s: csrs.vl == 0! vector instruction has nothing to do!\n", __func__);
        return;
    }

    //int64_t imm_se = (imm << (64 - 5)) >> (64 - 5); // sign extension
    int64_t imm_se = imm; // skip sign extension for now

    rv64v_vtype_t vtype;
    rv64_csr_decode_vtype(hart->csrs.vtype, &vtype);

    vector_register_t* vs2 = &hart->vector_registers[vs2_idx];
    vector_register_t* vd = &hart->vector_registers[vd_idx];

    for (int i = 0; i < hart->csrs.vl; i++) {
        bool mask_elem_active = true;
        if (vm == 0) {
            // todo: set mask_elem_active accordingly
        } else {
            mask_elem_active = true; // masking disabled, so always load everything
        }
        if (mask_elem_active) {
            switch(vtype.selected_element_width) {
                case RV64_SEW_8: {
                    vd->elements_8[i] = vs2->elements_8[i] + imm_se;
                    break;
                }
                case RV64_SEW_16: {
                    vd->elements_16[i] = vs2->elements_16[i] + imm_se;
                    break;
                }
                case RV64_SEW_32: {
                    vd->elements_32[i] = vs2->elements_32[i] + imm_se;
                    break;
                }
                case RV64_SEW_64: {
                    vd->elements_64[i] = vs2->elements_64[i] + imm_se;
                    break;
                }
            }
        }
    }
}

// todo

/*
 * MARK: Main
 */

emu_result_t rv64v_vector_emulate(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
) {
    printf("%s\n", __func__);

    // todo: clean this up, maybe make a cached helper csr with mstatus already decoded
    uint64_t mstatus_raw = rv64_get_csr_value(&hart->csrs, RV64_CSR_MSTATUS);
    uint8_t vs = (mstatus_raw >> 9) & 0b11;
    if (vs != 0) { // 0 = off, 1 = initial, 2, = clean, 3 = dirty
        // TODO: illegal instruction!
    } // todo: change to/from initial/clean/dirty. mstatus.sd is also modified. see: 30.3.2

    // RV64V
    switch (tag) {
        // vector admin/config
        case I_RV64V_VSETVLI: {
            rv64v_vsetvli(hart, raw_instruction, tag);
            break;
        }
        // case I_RV64V_VSETIVLI: {
        //     rv64v_emulate_vsetivli(hart, raw_instruction, tag);
        //     break;
        // }
        // case I_RV64V_VSETVL: {
        //     rv64v_emulate_vsetvl(hart, raw_instruction, tag);
        //     break;
        // }
        // load
        case I_RV64V_VLE8_V: {
            rv64v_vle8_v(hart, raw_instruction, tag);
            break;
        }
        case I_RV64V_VLE16_V: {
            rv64v_vle16_v(hart, raw_instruction, tag);
            break;
        }
        case I_RV64V_VLE32_V: {
            rv64v_vle32_v(hart, raw_instruction, tag);
            break;
        }
        // Constant stride...
        // case I_RV64V_VLSE8_V: {
        //     rv64v_vlse8_V(hart, raw_instruction, tag);
        //     break;
        // }
        // ...
        // store
        case I_RV64V_VSE8_V: {
            rv64v_vse8_v(hart, raw_instruction, tag);
            break;
        }
        // ...
        // arithmetic
        case I_RV64V_VADD_IVV: {
            rv64v_vadd_vv(hart, raw_instruction, tag);
            break;
        }
        case I_RV64V_VADD_IVX: {
            rv64v_vadd_vx(hart, raw_instruction, tag);
            break;
        }
        case I_RV64V_VADD_IVI: {
            rv64v_vadd_vi(hart, raw_instruction, tag);
            break;
        }

        // todo: the rest of the owl
        default: {
            return(ER_FAILURE);
        }
    }
    return(ER_SUCCESS);
}
