/**
 * fence tests
 * `fence`
 * "Fence" - Memory barrier
 * @see https://en.wikipedia.org/wiki/Memory_barrier
 */

#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_fence_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

Test(emu_rv64_emulate__fence__tests, fence_1, .init = rv64_emu_fence_default_setup)
{
    uint8_t input[] = {
        0x0f, 0x00, 0xf0, 0x0f, // fence
    };
    cr_assert(SUCCESS == rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input)));
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    // todo:
}