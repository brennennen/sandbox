
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_jal_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

Test(emu_rv64_emulate__jal__tests, jal_1, .init = rv64_emu_jal_default_setup)
{
    // li is a pseudo instruction, "li t1, 10" expands to "addi t1, zero, 10"
    uint8_t input[] = {
        0x13, 0x03, 0xa0, 0x00, // li t1, 10
        0xef, 0x00, 0x80, 0x00, // jal ra, next
        0x93, 0x03, 0x40, 0x01, // li t2, 20
                                // next:
        0x13, 0x0e, 0xe0, 0x01, // li t3, 30
    };
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(3 == g_emulator.harts[0].instructions_count);
    cr_assert(PROGRAM_START + 4 + 4 == g_emulator.harts[0].registers[RV64_REG_RA]);
    cr_assert(10 == g_emulator.harts[0].registers[RV64_REG_T1]);
    cr_assert(0 == g_emulator.harts[0].registers[RV64_REG_T2]); // we jumped over setting t2, so should be 0.
    cr_assert(30 == g_emulator.harts[0].registers[RV64_REG_T3]);
}
