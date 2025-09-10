#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_control_status_registers.h"
#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_vle32_v_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

/**
 * Load 4 "32 bit elements" into a vector registers (128 bits).
 */
Test(emu_rv64_emulate__vle32_v__tests, vle32_v_1, .init = rv64_emu_vle32_v_default_setup)
{
    // arrange
    // mock a vset config instruction: vsetvli t0, a2, e32, m1, ta, ma
    rv64v_vtype_t vtype = {
        .vma = 0, .vta = 0, .selected_element_width = RV64_SEW_32, .vlmul = RV64_VLMUL_1
    };
    g_emulator.csrs.vtype = rv64_csr_encode_vtype(&vtype);
    g_emulator.csrs.vl = 4; // 128 bit vlen, 32 bit elements, no grouping (vl = 128 / 32 = 4)

    uint32_t src_data[4];
    for (int i = 0; i < 4; i++) {
        src_data[i] = i;
    }
    memcpy(&g_emulator.memory[0x2000], src_data, sizeof(src_data));
    g_emulator.registers[RV64_REG_A1] = 0x2000;

    // 02 05 e0 07
    uint8_t input[] = { 0x07, 0xe0, 0x05, 0x02 }; // vle32.v v0, (a1)

    // act
    result_t result = emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input));

    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.instructions_count);
    for (int i = 0; i < 4; i++) {
        printf("%s: vreg[0][%d]: %d\n", __func__, i, g_emulator.vector_registers[0].elements_32[i]);
        cr_assert(i == g_emulator.vector_registers[0].elements_32[i]);
    }
}