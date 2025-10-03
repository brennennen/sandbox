
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/binary_utilities.h"
#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_fsw_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

Test(emu_rv64_emulate__fsw__tests, fsw_1, .init = rv64_emu_fsw_default_setup)
{
    // arrange
    size_t float_memory_address = PROGRAM_START + 0x64;
    g_emulator.harts[0].registers[RV64_REG_A0] = float_memory_address;
    const float pi_val = 3.14159f;
    g_emulator.harts[0].float32_registers[RV64_REF_FA0] = pi_val;
    uint8_t input[] = { 0x27, 0x20, 0xa5, 0x00 }; // fsw fa0, 0(a0)
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    float* float_value = (float*)&g_emulator.shared_system.memory[float_memory_address];
    cr_assert_float_eq(pi_val, *float_value, 1e-6);
}
