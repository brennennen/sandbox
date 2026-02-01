
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "8086/instruction_tags_8086.h"

#include "8086/emulate_8086.h"

//
// MARK: CMP
//

static emulator_8086_t g_emulator;

void emu_cmp_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_8086_t));
    emu_8086_init(&g_emulator);
}

// MARK: 1. I_CMP Tests
Test(emu__I_CMP__tests, cmp_1, .init = emu_cmp_default_setup) {
    uint8_t input[] = {0x39, 0xd6};  // cmp si, dx
    g_emulator.registers.si = 10;
    g_emulator.registers.dx = 10;
    cr_assert(SUCCESS == emu_8086_emulate_chunk(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(1 == (g_emulator.registers.flags & FLAG_ZF_MASK) >> FLAG_ZF_BIT_OFFSET);
    cr_assert(10 == g_emulator.registers.si);
    cr_assert(10 == g_emulator.registers.dx);
}

Test(emu__I_CMP__tests, cmp_2, .init = emu_cmp_default_setup) {
    uint8_t input[] = {0x39, 0xd6};  // cmp si, dx
    g_emulator.registers.si = 11;
    g_emulator.registers.dx = 10;
    cr_assert(SUCCESS == emu_8086_emulate_chunk(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(0 == (g_emulator.registers.flags & FLAG_ZF_MASK));
    cr_assert(11 == g_emulator.registers.si);
    cr_assert(10 == g_emulator.registers.dx);
}
