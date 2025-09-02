#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_vle8_v_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

Test(emu_rv64_emulate__vle8_v__tests, vle8_v_1, .init = rv64_emu_vle8_v_default_setup)
{
    // todo: set vector config stuff: vsetvli t0, a2, e8, m8, ta, ma
    // mock a vset config instruction: vsetvli t0, a2, e8, m8, ta, ma
    g_emulator.csrs.vl = 16; // e8
    // todo: set vtype accordingly (and other vector related crs?)

    uint8_t src_data[16];
    for (int i = 0; i < 16; i++) {
        src_data[i] = i;
        printf("src_data[%d] = %d\n", i, i);
    }
    memcpy(&g_emulator.memory[0x2000], src_data, sizeof(src_data));
    g_emulator.registers.regs[RV64_REG_A0] = 0x2000;
    uint8_t input[] = { 0x07, 0x00, 0x05, 0x02 }; // vle8.v v0, (a0)
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(1 == g_emulator.instructions_count);
    for (int i = 0; i < 16; i++) {
        src_data[i] = i;
        printf("vregs[0][%d] = %d\n", i, g_emulator.registers.vregs[0][i]);
        cr_assert(i == g_emulator.registers.vregs[0][i]);
    }
}
