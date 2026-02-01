/**
 * ebreak tests
 * `ebreak`
 * "Environment Break" - Return control to a debugging environment.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#ecall-ebreak
 */

#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_ebreak_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

Test(emu_rv64_emulate__ebreak__tests, ebreak_1, .init = rv64_emu_ebreak_default_setup) {
    uint8_t input[] = {
        0x73,
        0x00,
        0x10,
        0x00,  // ebreak
    };
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    // todo:
}
