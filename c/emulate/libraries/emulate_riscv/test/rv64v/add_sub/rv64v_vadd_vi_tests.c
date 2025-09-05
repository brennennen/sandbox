#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_control_status_registers.h"
#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_vadd_vi_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

Test(emu_rv64_emulate__vadd_vi__tests, vadd_vi_1, .init = rv64_emu_vadd_vi_default_setup)
{
    // mock a vset config instruction: vsetvli t0, a2, e8, m1, ta, ma
    rv64v_vtype_t vtype = {
        .vma = 0, .vta = 0, .selected_element_width = RV64_SEW_8, .vlmul = RV64_VLMUL_1
    };
    g_emulator.csrs.vtype = rv64_csr_encode_vtype(&vtype);
    g_emulator.csrs.vl = 16; // 128 bit vlen, 8 bit elements, no grouping (vl = 128 / 8 = 16)

    uint8_t src_data[16];
    for (int i = 0; i < 16; i++) {
        src_data[i] = i;
        printf("vs1[%d] = %d\n", i, i);
        g_emulator.vector_registers[1].elements_8[i] = i;
    }
    memcpy(&g_emulator.memory[0x2000], src_data, sizeof(src_data));
    g_emulator.registers[RV64_REG_A0] = 0x2000;
    uint8_t input[] = { 0x57, 0x31, 0x16, 0x02 }; // vadd.vi v2, v1, 12
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(1 == g_emulator.instructions_count);
    for (int i = 0; i < 16; i++) {
        src_data[i] = i;
        printf("vd[%d] = %d\n", i, g_emulator.vector_registers[2].elements_8[i]);
        cr_assert(i + 12 == g_emulator.vector_registers[2].elements_8[i]);
    }
}
