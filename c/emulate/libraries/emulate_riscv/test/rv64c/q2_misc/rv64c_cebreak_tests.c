#include <criterion/criterion.h>
#include <string.h>
#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_cebreak_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
    g_emulator.harts[0].rv64c_enabled = true;
}

Test(emu_rv64c_emulate__cebreak__tests, cebreak_1, .init = rv64_emu_cebreak_default_setup) {
    // TODO
    // cr_assert(1 == 0);
}
