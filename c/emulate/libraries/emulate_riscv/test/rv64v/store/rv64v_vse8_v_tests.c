#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_control_status_registers.h"
#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_vse8_v_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

Test(emu_rv64_emulate__vse8_v__tests, vse8_v_1, .init = rv64_emu_vse8_v_default_setup)
{
    // mock a vset config instruction: vsetvli t0, a2, e8, m1, ta, ma
    rv64v_vtype_t vtype = {
        .vma = 0, .vta = 0, .selected_element_width = RV64_SEW_8, .vlmul = RV64_VLMUL_1
    };
    g_emulator.harts[0].csrs.vtype = rv64_csr_encode_vtype(&vtype);
    g_emulator.harts[0].csrs.vl = 16; // 128 bit vlen, 8 bit elements, no grouping (vl = 128 / 8 = 16)

    uint8_t src_data[16];
    for (int i = 0; i < 16; i++) {
        src_data[i] = i;
    }
    memcpy(&g_emulator.harts[0].vector_registers[0], src_data, sizeof(src_data));
    g_emulator.harts[0].registers[RV64_REG_A1] = 0x2000;

    uint8_t input[] = { 0x27, 0x80, 0x05, 0x02 }; // vse8.v v0, (a1)
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));

    cr_assert(1 == g_emulator.harts[0].instructions_count);
    for (int i = 0; i < 16; i++) {
        src_data[i] = i;

        cr_assert(i == g_emulator.harts[0].shared_system->memory[0x2000 + i]);
    }
}
