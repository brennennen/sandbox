#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"

#include "libraries/emulate_intel/include/emulate.h"

static emulator_t g_emulator;

void emu_i386_decode_and_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_t));
    emu_init(&g_emulator);
    g_emulator.bits_mode = BITS_32;
}

Test(decode_i386__I_AND__tests, and__register__m32, .init = emu_i386_decode_and_default_setup)
{
    char* expected = "and ebx, ecx\n";
    uint8_t input[] = { 0x21, 0xcb }; // bits 32 "and ebx, ecx" - 0b00100001 0b11001011
    char output[32] = { 0x00 };
    cr_assert(SUCCESS == emu_decode_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}
