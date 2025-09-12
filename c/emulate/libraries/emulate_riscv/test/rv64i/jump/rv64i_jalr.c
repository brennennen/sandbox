
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_jalr_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

Test(emu_rv64_emulate__jalr__tests, jalr_1, .init = rv64_emu_jalr_default_setup)
{
    uint8_t input[] = {
        0x13, 0x03, 0xa0, 0x00, // li t1, 10
        0xef, 0x00, 0xc0, 0x00, // jal ra, next_section
        0x93, 0x03, 0x40, 0x01, // li t2, 20
        0xef, 0x00, 0xc0, 0x00, // jal ra, end
                                // next
        0x13, 0x0e, 0xe0, 0x01, // li t3, 30
        0x67, 0x80, 0x00, 0x00  // jalr x0, ra, 0
                                // end
    };
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(6 == g_emulator.harts[0].instructions_count);
    cr_assert(10 == g_emulator.harts[0].registers[RV64_REG_T1]);
    cr_assert(20 == g_emulator.harts[0].registers[RV64_REG_T2]);
    cr_assert(30 == g_emulator.harts[0].registers[RV64_REG_T3]);
}
