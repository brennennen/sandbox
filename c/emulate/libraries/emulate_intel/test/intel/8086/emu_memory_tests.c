/**
 * Emulation tests for the "add" instruction.
 */
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "8086/instruction_tags_8086.h"
#include "8086/emulate_8086.h"
#include "8086/emu_8086_registers.h"

static emulator_8086_t g_emulator;
void emu_memory_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_8086_t));
    emu_8086_init(&g_emulator);
}

Test(emu__memory__tests, memory_get_byte, .init = emu_memory_default_setup)
{
    g_emulator.memory[1000] = 0x08;
    uint8_t actual = 0;
    emu_result_t result = emu_memory_get_byte(&g_emulator, 1000, &actual);
    cr_assert(SUCCESS == result);
    cr_assert(8 == actual);
}

Test(emu__memory__tests, memory_get_uint16, .init = emu_memory_default_setup)
{
    g_emulator.memory[1000] = 0x00; // 256
    g_emulator.memory[1001] = 0x01;
    uint16_t actual = 0;
    emu_result_t result = emu_memory_get_uint16(&g_emulator, 1000, &actual);
    cr_assert(SUCCESS == result);
    cr_assert(256 == actual);
}
