#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_control_status_registers.h"
#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_vle16_v_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

/**
 * Load 8 "16 bit elements" into a vector register (128 bits).
 */
Test(emu_rv64_emulate__vle16_v__tests, vle16_v_1, .init = rv64_emu_vle16_v_default_setup)
{
    // arrange
    // mock a vset config instruction: vsetvli t0, a2, e16, m1, ta, ma
    rv64v_vtype_t vtype = {
        .vma = 0, .vta = 0, .selected_element_width = RV64_SEW_16, .vlmul = RV64_VLMUL_1
    };
    g_emulator.csrs.vtype = rv64_csr_encode_vtype(&vtype);
    g_emulator.csrs.vl = 8; // 128 bit vlen, 16 bit elements, no grouping (vl = 128 / 16 = 8)

    uint16_t src_data[8];
    for (int i = 0; i < 8; i++) {
        src_data[i] = i;
        printf("%s: src_data[%d]: %d\n", __func__, i, src_data[i]);
    }
    memcpy(&g_emulator.memory[0x2000], src_data, sizeof(src_data));
    g_emulator.registers[RV64_REG_A1] = 0x2000;
    uint8_t input[] = { 0x07, 0xd0, 0x05, 0x02 }; // vle16.v v0, (a1)

    // act
    result_t result = emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input));

    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    for (int i = 0; i < 8; i++) {
        printf("%s: g_emulator.vector_registers[0].elements_16[%d]: %d\n",
            __func__, i, g_emulator.vector_registers[0].elements_16[i]);
        cr_assert(i == g_emulator.vector_registers[0].elements_16[i]);
    }
}

/**
 * Load 16 "16 bit elements" into 2 vector registers (128 bits each, 256 bits total).
 */
Test(emu_rv64_emulate__vle16_v__tests, vle16_v_vlmul2, .init = rv64_emu_vle16_v_default_setup)
{
    // arrange
    // mock a vset config instruction: vsetvli t0, a2, e16, m2, ta, ma
    rv64v_vtype_t vtype = {
        .vma = 0, .vta = 0, .selected_element_width = RV64_SEW_16, .vlmul = RV64_VLMUL_2
    };
    g_emulator.csrs.vtype = rv64_csr_encode_vtype(&vtype);
    uint16_t vreg_elem_count = (VLEN / 16); // how many elements per vector register (8)
    uint16_t total_elem_count = (VLEN / 16) * 2; // total number of elements operated on (16) (2 vector registers with 8 elements each)
    g_emulator.csrs.vl = total_elem_count; // 128 bit vlen, 16 bit elements, group of 2 (vl = 128 / 16 * 2 = 16)

    uint16_t src_data[total_elem_count];
    for (int i = 0; i < total_elem_count; i++) {
        src_data[i] = i;
    }
    memcpy(&g_emulator.memory[0x2000], src_data, sizeof(src_data));
    g_emulator.registers[RV64_REG_A1] = 0x2000;
    uint8_t input[] = { 0x07, 0xd0, 0x05, 0x02 }; // vle16.v v0, (a1)

    // act
    result_t result = emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input));

    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    for (int i = 0; i < vreg_elem_count; i++) {
        cr_assert(i == g_emulator.vector_registers[0].elements_16[i]);
        cr_assert(i + (vreg_elem_count) == g_emulator.vector_registers[1].elements_16[i]);
    }
}

/**
 * Load 32 "16 bit elements" into 4 vector registers (128 bits each, 512 bits total).
 */
Test(emu_rv64_emulate__vle16_v__tests, vle16_v_vlmul4, .init = rv64_emu_vle16_v_default_setup)
{
    // arrange
    // mock a vset config instruction: vsetvli t0, a2, e16, m2, ta, ma
    rv64v_vtype_t vtype = {
        .vma = 0, .vta = 0, .selected_element_width = RV64_SEW_16, .vlmul = RV64_VLMUL_4
    };
    g_emulator.csrs.vtype = rv64_csr_encode_vtype(&vtype);
    uint16_t vreg_elem_count = (VLEN / 16); // how many elements per vector register (8)
    uint16_t total_elem_count = (VLEN / 16) * 4; // total number of elements operated on (32) (2 vector registers with 8 elements each)
    g_emulator.csrs.vl = total_elem_count; // 128 bit vlen, 16 bit elements, group of 4 (vl = 128 / 16 * 4 = 32)

    uint16_t src_data[total_elem_count];
    for (int i = 0; i < total_elem_count; i++) {
        src_data[i] = i;
    }
    memcpy(&g_emulator.memory[0x2000], src_data, sizeof(src_data));
    g_emulator.registers[RV64_REG_A1] = 0x2000;
    uint8_t input[] = { 0x07, 0xd0, 0x05, 0x02 }; // vle16.v v0, (a1)

    // act
    result_t result = emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input));

    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    for (int i = 0; i < vreg_elem_count; i++) {
        cr_assert(i == g_emulator.vector_registers[0].elements_16[i]);
        cr_assert(i + (vreg_elem_count) == g_emulator.vector_registers[1].elements_16[i]);
        cr_assert(i + (vreg_elem_count * 2) == g_emulator.vector_registers[2].elements_16[i]);
        cr_assert(i + (vreg_elem_count * 3) == g_emulator.vector_registers[3].elements_16[i]);
    }
}

/**
 * Load 64 "16 bit elements" into 8 vector registers (128 bits each, 1024 bits total).
 */
Test(emu_rv64_emulate__vle16_v__tests, vle16_v_vlmul8, .init = rv64_emu_vle16_v_default_setup)
{
    // arrange
    // mock a vset config instruction: vsetvli t0, a2, e16, m2, ta, ma
    rv64v_vtype_t vtype = {
        .vma = 0, .vta = 0, .selected_element_width = RV64_SEW_16, .vlmul = RV64_VLMUL_8
    };
    g_emulator.csrs.vtype = rv64_csr_encode_vtype(&vtype);
    uint16_t vreg_elem_count = (VLEN / 16); // how many elements per vector register (8)
    uint16_t total_elem_count = (VLEN / 16) * 8; // total number of elements operated on (64) (2 vector registers with 8 elements each)
    g_emulator.csrs.vl = total_elem_count; // 128 bit vlen, 16 bit elements, group of 2 (vl = 128 / 16 * 2 = 16)

    uint16_t src_data[total_elem_count];
    for (int i = 0; i < total_elem_count; i++) {
        src_data[i] = i;
    }
    memcpy(&g_emulator.memory[0x2000], src_data, sizeof(src_data));
    g_emulator.registers[RV64_REG_A1] = 0x2000;
    uint8_t input[] = { 0x07, 0xd0, 0x05, 0x02 }; // vle16.v v0, (a1)

    // act
    result_t result = emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input));

    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    for (int i = 0; i < vreg_elem_count; i++) {
        cr_assert(i == g_emulator.vector_registers[0].elements_16[i]);
        cr_assert(i + (vreg_elem_count) == g_emulator.vector_registers[1].elements_16[i]);
        cr_assert(i + (vreg_elem_count * 2) == g_emulator.vector_registers[2].elements_16[i]);
        cr_assert(i + (vreg_elem_count * 3) == g_emulator.vector_registers[3].elements_16[i]);
        cr_assert(i + (vreg_elem_count * 4) == g_emulator.vector_registers[4].elements_16[i]);
        cr_assert(i + (vreg_elem_count * 5) == g_emulator.vector_registers[5].elements_16[i]);
        cr_assert(i + (vreg_elem_count * 6) == g_emulator.vector_registers[6].elements_16[i]);
        cr_assert(i + (vreg_elem_count * 7) == g_emulator.vector_registers[7].elements_16[i]);
    }
}
