
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"
#include "rv64/rv64_disassemble.h"

static emulator_rv64_t g_emulator;

void rv64m_disassemble_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

#define ASSERT_STR_WITH_LOG(expected, actual, max_count) \
    cr_assert(strncmp(expected, actual, max_count) == 0, \
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, actual);

// MARK: I_RV64M_MUL Tests
Test(emu_rv64_disassemble__I_RV64M_MUL__tests, mul_1, .init = rv64m_disassemble_default_setup)
{
    char* expected = "mul t0, t1, t2\n";
    uint8_t input[] = { 0x02, 0x73, 0x02, 0xb3 };
    char output[32] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(1 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}
