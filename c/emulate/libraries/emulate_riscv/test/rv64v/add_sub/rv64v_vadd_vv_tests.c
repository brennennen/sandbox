#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_control_status_registers.h"
#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_vadd_vv_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

/*
 * MARK: 8 bit SEW
 */
// add a scalar to 16 "8 bit elements" across 1 vector register
Test(emu_rv64_emulate__vadd_vv__tests, vadd_vv_sew8_vlmul1, .init = rv64_emu_vadd_vv_default_setup)
{
    // arrange
    g_emulator.harts[0].registers[RV64_REG_A1] = 12;
    rv64v_vtype_t vtype = { // mock: vsetvli t0, a2, e8, m1, ta, ma
        .vma = 0, .vta = 0, .selected_element_width = RV64_SEW_8, .vlmul = RV64_VLMUL_1
    };
    g_emulator.harts[0].csrs.vtype = rv64_csr_encode_vtype(&vtype);
    uint16_t elements_count = VLEN / 8; // 16 elements (assuming 128 bit vlen)
    g_emulator.harts[0].csrs.vl = elements_count;
    for (int i = 0; i < elements_count; i++) { // pre-populate registers with an incrementing value 1, 2, 3, ...
        g_emulator.harts[0].vector_registers[0].elements_8[i] = i;
        g_emulator.harts[0].vector_registers[8].elements_8[i] = i * 2;
    }
    g_emulator.harts[0].registers[RV64_REG_A0] = 0x2000;
    uint8_t input[] = { 0x57, 0x08, 0x04, 0x02 }; // vadd.vv v16, v0, v8
    // act
    result_t result = rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    for (int i = 0; i < elements_count; i++) {
        printf("%s: %d\n", __func__, g_emulator.harts[0].vector_registers[16].elements_8[i]);
        cr_assert(i * 3 == g_emulator.harts[0].vector_registers[16].elements_8[i]);
    }
}