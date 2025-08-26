#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_emulate.h"
#include "rv64/rv64_disassemble.h"

static emulator_rv64_t g_emulator;

void rv64v_disassemble_snippets_default_setup(void) {
    memset(&g_emulator, 0, sizeof(emulator_rv64_t));
    emu_rv64_init(&g_emulator);
}

#define ASSERT_STR_WITH_LOG(expected, actual, max_count) \
    cr_assert(strncmp(expected, actual, max_count) == 0, \
        "expected:\n'%s'\n\nactual:\n'%s'\n", expected, actual);

/**
 * https://github.com/riscvarchive/riscv-v-spec/blob/master/example/memcpy.s
 * riscv64-unknown-elf-as -march=rv64gv -o memcpy.o memcpy.asm
 * riscv64-unknown-elf-objcopy -O binary -j .text memcpy.o memcpy.text
 * xxd -i memcpy.text
 */
Test(emu_rv64v_disassemble_vector_snippet_tests, example_memcpy, .init = rv64v_disassemble_snippets_default_setup)
{
    /**
     * note: pseudo mnemonics/instructions like mov, ret, bnez not implemented (yet?)
     * also jumping to labels currently disassembles to relative offsets (instead of
     * injecting labels).
     */
    char* expected = "addi a3, a0, 0\n\
vsetvli t0, a2, e8, m8, ta, ma\n\
vle8.v v0, (a1)\n\
add a1, a1, t0\n\
sub a2, a2, t0\n\
vse8.v v0, (a3)\n\
add a3, a3, t0\n\
bne a2, zero, . + -24\n\
jalr zero, ra, 0\n";
    uint8_t input[] = {
        0x93, 0x06, 0x05, 0x00, 0xd7, 0x72, 0x36, 0x0c, 0x07, 0x80, 0x05, 0x02,
        0xb3, 0x85, 0x55, 0x00, 0x33, 0x06, 0x56, 0x40, 0x27, 0x80, 0x06, 0x02,
        0xb3, 0x86, 0x56, 0x00, 0xe3, 0x14, 0x06, 0xfe, 0x67, 0x80, 0x00, 0x00
    };
    char output[256] = { '\0' };
    cr_assert(SUCCESS == emu_rv64_disassemble_chunk(
        &g_emulator, input, sizeof(input), output, sizeof(output)));
    cr_assert(9 == g_emulator.instructions_count);
    ASSERT_STR_WITH_LOG(expected, output, sizeof(output));
}