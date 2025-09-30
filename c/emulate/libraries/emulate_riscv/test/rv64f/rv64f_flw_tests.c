
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/binary_utilities.h"
#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_flw_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

Test(emu_rv64_emulate__flw__tests, flw_1, .init = rv64_emu_flw_default_setup)
{
    // arrange
    size_t float_memory_address = PROGRAM_START + 0x64;
    g_emulator.harts[0].registers[RV64_REG_A0] = float_memory_address;
    const float pi_val = 3.14159f;
    memcpy(&g_emulator.shared_system.memory[float_memory_address], &pi_val, sizeof(float));
    uint8_t input[] = { 0x07, 0x25, 0x05, 0x00 }; // flw fa0, 0(a0)
    // act
    result_t result = rv64_hart_emulate_chunk(&g_emulator.harts[0], PROGRAM_START, input, sizeof(input));
    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    cr_assert_float_eq(pi_val, g_emulator.harts[0].float32_registers[10], 1e-6);
}
