#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_control_status_registers.h"
#include "rv64/rv64_emulate.h"

static rv64_emulator_t g_emulator;

void rv64_emu_vle8_v_default_setup(void) {
    memset(&g_emulator, 0, sizeof(rv64_emulator_t));
    rv64_emulator_init(&g_emulator);
}

/**
 * load 16 bytes into 1 vector register
 * (VLEN = 128 bits, 16 bytes per register vector)
 */
Test(emu_rv64_emulate__vle8_v__tests, vle8_v_1, .init = rv64_emu_vle8_v_default_setup) {
    // arrange
    rv64v_vtype_t vtype = {
        // mock: vsetvli t0, a2, e8, m1, ta, ma
        .vma = 0,
        .vta = 0,
        .selected_element_width = RV64_SEW_8,
        .vlmul = RV64_VLMUL_1
    };
    g_emulator.harts[0].csrs.vtype = rv64_csr_encode_vtype(&vtype);
    g_emulator.harts[0].csrs.vl =
        VLEN_BYTES;  // 128 bit vlen, 8 bit elements, no grouping (vl = 128 / 8 = 16)

    uint8_t src_data[VLEN_BYTES];
    for (int i = 0; i < VLEN_BYTES; i++) {
        src_data[i] = i;
    }
    memcpy(&g_emulator.harts[0].shared_system->memory[0x2000], src_data, sizeof(src_data));
    g_emulator.harts[0].registers[RV64_REG_A0] = 0x2000;
    uint8_t input[] = {0x07, 0x00, 0x05, 0x02};  // vle8.v v0, (a0)

    // act
    result_t result = rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input));

    // assert
    cr_assert(SUCCESS == result);

    cr_assert(1 == g_emulator.harts[0].instructions_count);
    for (int i = 0; i < VLEN_BYTES; i++) {
        src_data[i] = i;
        cr_assert(i == g_emulator.harts[0].vector_registers[0].bytes[i]);
    }
}

/**
 * load 32 bytes into 2 vector registers
 * vlmul = 2, use 2 vector registers
 */
Test(emu_rv64_emulate__vle8_v__tests, vle8_v_vlmul2, .init = rv64_emu_vle8_v_default_setup) {
    // arrange
    rv64v_vtype_t vtype = {
        // mock: vsetvli t0, a2, e8, m2, ta, ma
        .vma = 0,
        .vta = 0,
        .selected_element_width = RV64_SEW_8,
        .vlmul = RV64_VLMUL_2
    };
    g_emulator.harts[0].csrs.vtype = rv64_csr_encode_vtype(&vtype);
    // VLMUL = (VLEN / SEW) * LMUL = (128 / 8) * 2 = 32
    uint16_t bytes_count = VLEN_BYTES * 2;  // 2 full vlen registers
    g_emulator.harts[0].csrs.vl = bytes_count;

    uint8_t src_data[bytes_count];
    for (int i = 0; i < bytes_count; i++) {
        src_data[i] = i;
    }
    memcpy(&g_emulator.harts[0].shared_system->memory[0x2000], src_data, sizeof(src_data));
    g_emulator.harts[0].registers[RV64_REG_A0] = 0x2000;
    uint8_t input[] = {0x07, 0x00, 0x05, 0x02};  // vle8.v v0, (a0)

    // act
    result_t result = rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input));

    // assert
    cr_assert(SUCCESS == result);

    cr_assert(1 == g_emulator.harts[0].instructions_count);
    // with vlmul = 2, this instruction operates on v0 and v1.
    // 0 - 15 bytes are in v0
    for (int i = 0; i < VLEN_BYTES; i++) {
        cr_assert(i == g_emulator.harts[0].vector_registers[0].bytes[i]);
    }
    // 16 - 31 bytes are in v1
    for (int i = 0; i < VLEN_BYTES; i++) {
        cr_assert(i + VLEN_BYTES == g_emulator.harts[0].vector_registers[1].bytes[i]);
    }
}

/**
 * load 64 bytes into 4 vector registers
 * vlmul = 4, use 4 vector registers
 */
Test(emu_rv64_emulate__vle8_v__tests, vle8_v_vlmul4, .init = rv64_emu_vle8_v_default_setup) {
    // arrange
    rv64v_vtype_t vtype = {
        // mock: vsetvli t0, a2, e8, m4, ta, ma
        .vma = 0,
        .vta = 0,
        .selected_element_width = RV64_SEW_8,
        .vlmul = RV64_VLMUL_4
    };
    g_emulator.harts[0].csrs.vtype = rv64_csr_encode_vtype(&vtype);
    // VLMUL = (VLEN / SEW) * LMUL = (128 / 8) * 4 = 64
    uint16_t bytes_count = VLEN_BYTES * 4;
    g_emulator.harts[0].csrs.vl = bytes_count;

    uint8_t src_data[bytes_count];
    for (int i = 0; i < bytes_count; i++) {
        src_data[i] = i;
    }
    memcpy(&g_emulator.harts[0].shared_system->memory[0x2000], src_data, sizeof(src_data));
    g_emulator.harts[0].registers[RV64_REG_A0] = 0x2000;
    uint8_t input[] = {0x07, 0x00, 0x05, 0x02};  // vle8.v v0, (a0)

    // act
    result_t result = rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input));

    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    // with vlmul = 2, this instruction operates on v0, v1, v2, and v3
    // 0 - 15 bytes are in v0, 16 - 31 bytes are in v1, etc.
    for (int i = 0; i < VLEN_BYTES; i++) {
        cr_assert(i == g_emulator.harts[0].vector_registers[0].bytes[i]);
        cr_assert(i + VLEN_BYTES == g_emulator.harts[0].vector_registers[1].bytes[i]);
        cr_assert((i + (VLEN_BYTES * 2)) == g_emulator.harts[0].vector_registers[2].bytes[i]);
        cr_assert((i + (VLEN_BYTES * 3)) == g_emulator.harts[0].vector_registers[3].bytes[i]);
    }
}

/**
 * load 128 bytes into 8 vector registers
 * vlmul = 8, use 8 vector registers
 */
Test(emu_rv64_emulate__vle8_v__tests, vle8_v_vlmul8, .init = rv64_emu_vle8_v_default_setup) {
    // arrange
    rv64v_vtype_t vtype = {
        // mock: vsetvli t0, a2, e8, m4, ta, ma
        .vma = 0,
        .vta = 0,
        .selected_element_width = RV64_SEW_8,
        .vlmul = RV64_VLMUL_8
    };
    g_emulator.harts[0].csrs.vtype = rv64_csr_encode_vtype(&vtype);
    // VLMUL = (VLEN / SEW) * LMUL = (128 / 8) * 8 = 128
    uint16_t bytes_count = VLEN_BYTES * 8;
    g_emulator.harts[0].csrs.vl = bytes_count;

    uint8_t src_data[bytes_count];
    for (int i = 0; i < bytes_count; i++) {
        src_data[i] = i;
    }
    memcpy(&(g_emulator.harts[0].shared_system->memory[0x2000]), src_data, sizeof(src_data));
    g_emulator.harts[0].registers[RV64_REG_A0] = 0x2000;
    uint8_t input[] = {0x07, 0x00, 0x05, 0x02};  // vle8.v v0, (a0)

    // act
    result_t result = rv64_emulate_chunk_single_core(&g_emulator, input, sizeof(input));

    // assert
    cr_assert(SUCCESS == result);
    cr_assert(1 == g_emulator.harts[0].instructions_count);
    // with vlmul = 8, this instruction operates on v0 thru v7
    // 0 - 15 bytes are in v0, 16 - 31 bytes are in v1, etc.
    for (int i = 0; i < VLEN_BYTES; i++) {
        cr_assert(i == g_emulator.harts[0].vector_registers[0].bytes[i]);
        cr_assert((i + VLEN_BYTES) == g_emulator.harts[0].vector_registers[1].bytes[i]);
        cr_assert((i + (VLEN_BYTES * 2)) == g_emulator.harts[0].vector_registers[2].bytes[i]);
        cr_assert((i + (VLEN_BYTES * 3)) == g_emulator.harts[0].vector_registers[3].bytes[i]);
        cr_assert((i + (VLEN_BYTES * 4)) == g_emulator.harts[0].vector_registers[4].bytes[i]);
        cr_assert((i + (VLEN_BYTES * 5)) == g_emulator.harts[0].vector_registers[5].bytes[i]);
        cr_assert((i + (VLEN_BYTES * 6)) == g_emulator.harts[0].vector_registers[6].bytes[i]);
        cr_assert((i + (VLEN_BYTES * 7)) == g_emulator.harts[0].vector_registers[7].bytes[i]);
    }
}
