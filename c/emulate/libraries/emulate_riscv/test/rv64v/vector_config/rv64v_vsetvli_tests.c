#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_vsetvli_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

Test(emu_rv64_emulate__vsetvli__tests, vsetvli_1, .init = rv64_emu_vsetvli_default_setup) {
    g_emulator.harts[0].csrs.vtype = 0;
    g_emulator.harts[0].registers[RV64_REG_A2] = 8;  // rs1 = avl
    uint8_t input[] = {0xd7, 0x72, 0x36, 0x0c};      // vsetvli t0, a2, e8, m8, ta, ma
    // vtype bits: 20 - 30 = 00 1100 0011
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));

    cr_assert(1 == g_emulator.harts[0].instructions_count);
    // cr_assert(0 == g_emulator.harts[0].registers[RV64_REG_T0]); // TODO: this might not be right,
    // revise what to return here
    uint8_t vma = (g_emulator.harts[0].csrs.vtype << 7) & 0b1;
    cr_assert(0 == vma);  // ma = 0 = Mask agnostic
    uint8_t vta = (g_emulator.harts[0].csrs.vtype << 6) & 0b1;
    cr_assert(0 == vta);  // ta = 0 = Tail agnostic
    uint8_t vsew = (g_emulator.harts[0].csrs.vtype << 3) & 0b111;
    cr_assert(0 == vsew);  // vsew = e8 = 0 = 0b000
    uint8_t vlmul = g_emulator.harts[0].csrs.vtype & 0b111;
    cr_assert(3 == vlmul);  // vlmul = m8 = 3 = 0b011
    cr_assert(8 == g_emulator.harts[0].csrs.vl);
}
