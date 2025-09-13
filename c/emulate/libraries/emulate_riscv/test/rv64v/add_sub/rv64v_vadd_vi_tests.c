#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_control_status_registers.h"
#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_vadd_vi_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

/*
 * MARK: 8 bit SEW
 */
// add an immediate to 16 "8 bit elements" across 1 vector register
Test(emu_rv64_emulate__vadd_vi__tests, vadd_sew8_vlmul1, .init = rv64_emu_vadd_vi_default_setup)
{
    // arrange
    rv64v_vtype_t vtype = { // mock: vsetvli t0, a2, e8, m1, ta, ma
        .vma = 0, .vta = 0, .selected_element_width = RV64_SEW_8, .vlmul = RV64_VLMUL_1
    };
    g_emulator.harts[0].csrs.vtype = rv64_csr_encode_vtype(&vtype);
    uint16_t elements_count = VLEN / 8; // 16 elements (assuming 128 bit vlen)
    g_emulator.harts[0].csrs.vl = elements_count;
    for (int i = 0; i < elements_count; i++) { // pre-populate registers with an incrementing value 1, 2, 3, ...
        g_emulator.harts[0].vector_registers[1].elements_8[i] = i;
    }
    g_emulator.harts[0].registers[RV64_REG_A0] = 0x2000;
    uint8_t input[] = { 0x57, 0x31, 0x16, 0x02 }; // vadd.vi v2, v1, 12
    // act
    result_t result = rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    for (int i = 0; i < elements_count; i++) {
        cr_assert(i + 12 == g_emulator.harts[0].vector_registers[2].elements_8[i]);
    }
}

/*
 * MARK: 16 bit SEW
 */
// add an immediate to 8 "16 bit elements" across 1 vector register
Test(emu_rv64_emulate__vadd_vi__tests, vadd_vi_sew16_vlmul1, .init = rv64_emu_vadd_vi_default_setup)
{
    // arrange
    rv64v_vtype_t vtype = { // mock: vsetvli t0, a2, e16, m1, ta, ma
        .vma = 0, .vta = 0, .selected_element_width = RV64_SEW_16, .vlmul = RV64_VLMUL_1
    };
    g_emulator.harts[0].csrs.vtype = rv64_csr_encode_vtype(&vtype);
    uint16_t elements_count = VLEN / 16; // 8 elements (assuming 128 bit vlen)
    g_emulator.harts[0].csrs.vl = elements_count;
    for (int i = 0; i < elements_count; i++) { // pre-populate registers with an incrementing value 1, 2, 3, ...
        g_emulator.harts[0].vector_registers[1].elements_16[i] = i;
    }
    g_emulator.harts[0].registers[RV64_REG_A0] = 0x2000;
    uint8_t input[] = { 0x57, 0x31, 0x16, 0x02 }; // vadd.vi v2, v1, 12
    // act
    result_t result = rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    for (int i = 0; i < elements_count; i++) {
        cr_assert(i + 12 == g_emulator.harts[0].vector_registers[2].elements_16[i]);
    }
}

// add an immediate to 64 "16 bit elements" across 8 vector registers
Test(emu_rv64_emulate__vadd_vi__tests, vadd_vi_sew16_vlmul8, .init = rv64_emu_vadd_vi_default_setup)
{
    // arrange
    rv64v_vtype_t vtype = { // mock: vsetvli t0, a2, e16, m8, ta, ma
        .vma = 0, .vta = 0, .selected_element_width = RV64_SEW_16, .vlmul = RV64_VLMUL_8
    };
    g_emulator.harts[0].csrs.vtype = rv64_csr_encode_vtype(&vtype);
    uint16_t elem_per_vreg = VLEN / 16; // 64 elements (assuming 128 bit vlen)
    uint16_t elements_count = elem_per_vreg * 8;
    g_emulator.harts[0].csrs.vl = elements_count;
    for (int i = 0; i < elem_per_vreg; i++) {
        g_emulator.harts[0].vector_registers[0].elements_16[i] = i;
        g_emulator.harts[0].vector_registers[1].elements_16[i] = i + (1 * elem_per_vreg);
        g_emulator.harts[0].vector_registers[2].elements_16[i] = i + (2 * elem_per_vreg);
        g_emulator.harts[0].vector_registers[3].elements_16[i] = i + (3 * elem_per_vreg);
        g_emulator.harts[0].vector_registers[4].elements_16[i] = i + (4 * elem_per_vreg);
        g_emulator.harts[0].vector_registers[5].elements_16[i] = i + (5 * elem_per_vreg);
        g_emulator.harts[0].vector_registers[6].elements_16[i] = i + (6 * elem_per_vreg);
        g_emulator.harts[0].vector_registers[7].elements_16[i] = i + (7 * elem_per_vreg);
    }
    g_emulator.harts[0].registers[RV64_REG_A0] = 0x2000;
    uint8_t input[] = { 0x57, 0x34, 0x06, 0x02 }; // vadd.vi v8, v0, 12
    // act
    result_t result = rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    for (int i = 0; i < elem_per_vreg; i++) {
        cr_assert(i + 12 == g_emulator.harts[0].vector_registers[8].elements_16[i]);
        cr_assert(i + (1 * elem_per_vreg) + 12 == g_emulator.harts[0].vector_registers[9].elements_16[i]);
        cr_assert(i + (2 * elem_per_vreg) + 12 == g_emulator.harts[0].vector_registers[10].elements_16[i]);
        cr_assert(i + (3 * elem_per_vreg) + 12 == g_emulator.harts[0].vector_registers[11].elements_16[i]);
        cr_assert(i + (4 * elem_per_vreg) + 12 == g_emulator.harts[0].vector_registers[12].elements_16[i]);
        cr_assert(i + (5 * elem_per_vreg) + 12 == g_emulator.harts[0].vector_registers[13].elements_16[i]);
        cr_assert(i + (6 * elem_per_vreg) + 12 == g_emulator.harts[0].vector_registers[14].elements_16[i]);
        cr_assert(i + (7 * elem_per_vreg) + 12 == g_emulator.harts[0].vector_registers[15].elements_16[i]);
    }
}

/*
 * MARK: 32 bit SEW
 */
// add an immediate to 4 "32 bit elements" across 1 vector register
Test(emu_rv64_emulate__vadd_vi__tests, vadd_vi_sew32_vlmul1, .init = rv64_emu_vadd_vi_default_setup)
{
    // arrange
    rv64v_vtype_t vtype = { // mock: vsetvli t0, a2, e32, m1, ta, ma
        .vma = 0, .vta = 0, .selected_element_width = RV64_SEW_32, .vlmul = RV64_VLMUL_1
    };
    g_emulator.harts[0].csrs.vtype = rv64_csr_encode_vtype(&vtype);
    uint16_t elements_count = VLEN / 32; // 4 elements (assuming 128 bit vlen)
    g_emulator.harts[0].csrs.vl = elements_count; // 128 bit vlen, 32 bit elements, no grouping (vl = 128 / 32 = 4)
    for (int i = 0; i < elements_count; i++) {
        g_emulator.harts[0].vector_registers[1].elements_32[i] = i;
    }
    g_emulator.harts[0].registers[RV64_REG_A0] = 0x2000;
    uint8_t input[] = { 0x57, 0x31, 0x16, 0x02 }; // vadd.vi v2, v1, 12
    // act
    result_t result = rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    for (int i = 0; i < elements_count; i++) {
        cr_assert(i + 12 == g_emulator.harts[0].vector_registers[2].elements_32[i]);
    }
}

/*
 * MARK: 64 bit SEW
 */
// add an immediate to 2 "64 bit elements" across 1 vector register
Test(emu_rv64_emulate__vadd_vi__tests, vadd_vi_sew64_vlmul1, .init = rv64_emu_vadd_vi_default_setup)
{
    // arrange
    rv64v_vtype_t vtype = { // mock: vsetvli t0, a2, e32, m1, ta, ma
        .vma = 0, .vta = 0, .selected_element_width = RV64_SEW_64, .vlmul = RV64_VLMUL_1
    };
    g_emulator.harts[0].csrs.vtype = rv64_csr_encode_vtype(&vtype);
    uint16_t elements_count = VLEN / 64; // 2 elements (assuming 128 bit vlen)
    g_emulator.harts[0].csrs.vl = elements_count; // 128 bit vlen, 64 bit elements, no grouping (vl = 128 / 64 = 2)
    for (int i = 0; i < elements_count; i++) {
        g_emulator.harts[0].vector_registers[1].elements_64[i] = i;
    }
    g_emulator.harts[0].registers[RV64_REG_A0] = 0x2000;
    uint8_t input[] = { 0x57, 0x31, 0x16, 0x02 }; // vadd.vi v2, v1, 12
    // act
    result_t result = rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    for (int i = 0; i < elements_count; i++) {
        cr_assert(i + 12 == g_emulator.harts[0].vector_registers[2].elements_64[i]);
    }
}
