

#include <stdint.h>
#include <stdbool.h>

#include "shared/include/registers.h"

#include "libraries/emulate8086/include/emulate8086.h"
#include "libraries/emulate8086/include/emu_registers.h"


void print_registers(emulator_t* emulator) {
    printf("ax: %d\n\
bx: %d\n\
cx: %d\n\
dx: %d\n\
si: %d\n\
di: %d\n\
bp: %d\n\
sp: %d\n\
cs: %d\n\
ds: %d\n\
es: %d\n\
ss: %d\n\
ip: %d\n\
",
    emulator->registers.ax,
    emulator->registers.bx,
    emulator->registers.cx,
    emulator->registers.dx,
    emulator->registers.si,
    emulator->registers.di,
    emulator->registers.bp,
    emulator->registers.sp,
    emulator->registers.cs,
    emulator->registers.ds,
    emulator->registers.es,
    emulator->registers.ss,
    emulator->registers.ip
);
}

void print_flags(uint16_t flags) {
    printf("of: %d\n\
df: %d\n\
if: %d\n\
tf: %d\n\
sf: %d\n\
zf: %d\n\
af: %d\n\
pf: %d\n\
cf: %d\n\
",
    emu_reg_get_flag(flags, FLAG_OF_MASK) >> FLAG_CF_BIT_OFFSET,
    emu_reg_get_flag(flags, FLAG_DF_MASK) >> FLAG_DF_BIT_OFFSET,
    emu_reg_get_flag(flags, FLAG_IF_MASK) >> FLAG_IF_BIT_OFFSET,
    emu_reg_get_flag(flags, FLAG_TF_MASK) >> FLAG_TF_BIT_OFFSET,
    emu_reg_get_flag(flags, FLAG_SF_MASK) >> FLAG_SF_BIT_OFFSET,
    emu_reg_get_flag(flags, FLAG_ZF_MASK) >> FLAG_ZF_BIT_OFFSET,
    emu_reg_get_flag(flags, FLAG_AF_MASK) >> FLAG_AF_BIT_OFFSET,
    emu_reg_get_flag(flags, FLAG_PF_MASK) >> FLAG_PF_BIT_OFFSET,
    emu_reg_get_flag(flags, FLAG_CF_MASK) >> FLAG_CF_BIT_OFFSET
);
}

void emu_reg_update_carry_flag_8bit(uint16_t* flags, uint16_t uint16_result) {
    if (uint16_result >= 255) {
        emu_reg_set_flag(flags, FLAG_CF_MASK);
    } else {
        emu_reg_clear_flag(flags, FLAG_CF_MASK);
    }
}

void emu_reg_update_carry_flag_16bit(uint16_t* flags, uint16_t uint32_result) {
    if (uint32_result >= 65535) {
        emu_reg_set_flag(flags, FLAG_CF_MASK);
    } else {
        emu_reg_clear_flag(flags, FLAG_CF_MASK);
    }
}

void emu_reg_update_auxiliary_carry_flag(uint16_t* flags, uint16_t left, uint16_t right, uint16_t result) {
    // detect a carry in the least significant nibble.
    if (((left ^ right ^ result) & 0b1000) != 0) {
        emu_reg_set_flag(flags, FLAG_AF_MASK);
    } else {
        emu_reg_clear_flag(flags, FLAG_AF_MASK);
    }
}

void emu_reg_update_sign_flag_8bit(uint16_t* flags, uint8_t result) {
    if (result & 0b10000000) {
        emu_reg_set_flag(flags, FLAG_SF_MASK);
    } else {
        emu_reg_clear_flag(flags, FLAG_SF_MASK);
    }
}

void emu_reg_update_sign_flag_16bit(uint16_t* flags, uint16_t result) {
    if (result & 0b1000000000000000) {
        emu_reg_set_flag(flags, FLAG_SF_MASK);
    } else {
        emu_reg_clear_flag(flags, FLAG_SF_MASK);
    }
}

/**
 * c23 and greater should be using the optimized/builtin `stdc_count_ones_us`.
 * This is a last resort fallback and intentionally simple/unoptimized.
 */
uint16_t pop_count_uint16_fallback(uint16_t val) {
    int count = 0;
    while (val != 0) {
        count += val & 1;
        val = val >> 1;
    }
    return count;
}

void emu_reg_update_parity_flag(uint16_t* flags, uint16_t result) {
    if (pop_count_uint16(result & 0xFF) % 2 == 0) {
        emu_reg_set_flag(flags, FLAG_ZF_MASK);
    } else {
        emu_reg_clear_flag(flags, FLAG_ZF_MASK);
    }
}

void emu_reg_update_overflow_flag_8bit(uint16_t* flags, uint8_t left, uint8_t right, uint8_t arithmetic_result) {
    bool left_negative = (left & 0b10000000 != 0);
    bool right_negative = (right & 0b10000000 != 0);
    bool result_negative = (arithmetic_result & 0b10000000 != 0);
    if (left_negative == right_negative
        && left_negative != result_negative)
    {
        emu_reg_set_flag(flags, FLAG_OF_MASK);
    } else {
        emu_reg_clear_flag(flags, FLAG_OF_MASK);
    }
}

void emu_reg_update_overflow_flag_16bit(uint16_t* flags, uint16_t left, uint16_t right, uint16_t arithmetic_result) {
    bool left_negative = (left & 0x8000 != 0);
    bool right_negative = (right & 0x8000 != 0);
    bool result_negative = (arithmetic_result & 0x8000 != 0);
    if (left_negative == right_negative
        && left_negative != result_negative)
    {
        emu_reg_set_flag(flags, FLAG_OF_MASK);
    } else {
        emu_reg_clear_flag(flags, FLAG_OF_MASK);
    }
}
