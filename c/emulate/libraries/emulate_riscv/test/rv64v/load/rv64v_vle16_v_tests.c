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

Test(emu_rv64_emulate__vle16_v__tests, vle16_v_1, .init = rv64_emu_vle16_v_default_setup)
{
    // mock a vset config instruction: vsetvli t0, a2, e16, m1, ta, ma
    rv64v_vtype_t vtype = {
        .vma = 0, .vta = 0, .selected_element_width = RV64_SEW_16, .vlmul = RV64_VLMUL_1
    };
    g_emulator.csrs.vtype = rv64_csr_encode_vtype(&vtype);
    g_emulator.csrs.vl = 8; // 128 bit vlen, 16 bit elements, no grouping (vl = 128 / 16 = 8)

    uint8_t src_data[8];
    for (int i = 0; i < 8; i++) {
        src_data[i] = i;
        //printf("src_data[%d] = %d\n", i, i);
    }
    memcpy(&g_emulator.memory[0x2000], src_data, sizeof(src_data));
    g_emulator.registers[RV64_REG_A1] = 0x2000;
    uint8_t input[] = { 0x07, 0xd0, 0x05, 0x02 }; // vle16.v v0, (a1)
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(1 == g_emulator.instructions_count);
    for (int i = 0; i < 8; i++) {
        src_data[i] = i;
        //printf("vregs[0][%d] = %d\n", i, g_emulator.vector_registers[0].bytes[i]);
        cr_assert(i == g_emulator.vector_registers[0].bytes[i]);
    }
}
