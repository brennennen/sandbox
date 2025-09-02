#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_vsetvli_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

Test(emu_rv64_emulate__vsetvli__tests, vsetvli_1, .init = rv64_emu_vsetvli_default_setup)
{
    g_emulator.csrs.vtype = 0;
    uint8_t input[] = { 0xd7, 0x72, 0x36, 0x0c }; // vsetvli t0, a2, e8, m8, ta, ma
    // vtype bits: 20 - 30 = 00 1100 0011
    cr_assert(SUCCESS == emu_rv64_emulate_chunk(&g_emulator, input, sizeof(input)));
    debug_print_registers(&g_emulator);
    cr_assert(1 == g_emulator.instructions_count);
    //cr_assert(0 == g_emulator.registers.regs[RV64_REG_T0]); // TODO: this might not be right, revise what to return here
    uint8_t vma = (g_emulator.csrs.vtype << 7) & 0b1;
    cr_assert(0 == vma); // ma = 0 = Mask agnostic
    uint8_t vta = (g_emulator.csrs.vtype << 6) & 0b1;
    cr_assert(0 == vta); // ta = 0 = Tail agnostic
    uint8_t vsew = (g_emulator.csrs.vtype << 3) & 0b111;
    cr_assert(0 == vsew); // vsew = e8 = 0 = 0b000
    uint8_t vlmul = g_emulator.csrs.vtype & 0b111;
    cr_assert(3 == vlmul); // vlmul = m8 = 3 = 0b011
}
