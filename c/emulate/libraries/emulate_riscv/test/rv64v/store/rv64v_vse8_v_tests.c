#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_control_status_registers.h"
#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_vse8_v_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

Test(emu_rv64_emulate__vse8_v__tests, vse8_v_1, .init = rv64_emu_vse8_v_default_setup)
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
        printf("src_data[%d] = %d\n", i, i);
    }
    memcpy(&g_emulator.registers.vregs[0], src_data, sizeof(src_data));
    g_emulator.registers.regs[RV64_REG_A1] = 0x2000;

    uint8_t input[] = { 0x27, 0x80, 0x05, 0x02 }; // vse8.v v0, (a1)
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(1 == g_emulator.instructions_count);
    for (int i = 0; i < 16; i++) {
        src_data[i] = i;
        printf("g_emulator.memory[0x2000 + %d] = %d\n", i, g_emulator.memory[0x2000 + i]);
        cr_assert(i == g_emulator.memory[0x2000 + i]);
    }
}
