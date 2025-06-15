
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"

static emulator_rv64_t g_emulator;

void rv64_disassemble_register_register_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

// MARK: I_RV64I_ADD_REGISTER Tests
Test(emu_rv64_disassemble__I_RV64I_ADD__tests, add_1, .init = rv64_disassemble_register_register_default_setup)
{
    char* expected = "add t0, t1, t2\n";
    uint8_t input[] = { 0x00, 0x73, 0x02, 0xb3 }; // 007302b3
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    cr_assert(strncmp(expected, output, sizeof(output)) == 0,
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, output);
}
