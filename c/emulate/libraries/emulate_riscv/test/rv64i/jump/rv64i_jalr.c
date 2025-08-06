
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_emu_jalr_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

Test(emu_rv64_emulate__jalr__tests, jalr_1, .init = rv64_emu_jalr_default_setup)
{
    // TODO
    //cr_assert(0 == 1);
}
