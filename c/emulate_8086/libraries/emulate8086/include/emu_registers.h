
#ifndef EMU_REGISTERS
#define EMU_REGISTERS

#include <stdio.h>

#include "shared/include/result.h"
#include "shared/include/registers.h"

#include "libraries/emulate8086/include/emulate8086.h"

void print_registers(emulator_t* emulator);

void print_registers_condensed(emulator_t* emulator);

void print_flags(uint16_t flags);

void print_flags_condensed(uint16_t flags);

static inline void emu_reg_set_flag(uint16_t* flags, uint16_t mask) {
    *flags = *flags | mask;
}

static inline void emu_reg_clear_flag(uint16_t* flags, uint16_t mask) {
    *flags = *flags & ~mask;
}

static inline void emu_reg_toggle_flag(uint16_t* flags, uint16_t mask) {
    *flags = *flags ^ ~mask;
}

static inline uint16_t emu_reg_get_flag(uint16_t flags, uint16_t mask) {
    return flags & mask;
}

/**
 * Updates the carry flag in the flags register. Indicates the result isn't
 * mathematically correct when interpreted as unsigned.
 *
 * CF (carry flag): If an addition results in a carry out of the high-order bit
 * of the result, then CF is set; otherwise CF is cleared. If a subtraction
 * results in a borrow into the high-order bit of the result, then CF is set;
 * otherwise CF is cleared. Note that a signed carry is indicated by CF != OF.
 * CF can be used to detect an unsigned overflow. Two instructions, ADC (add
 * with carry) and SBB (subtract with borrow), incorporate the carry flag in
 * their operations and can be used to perform multi-byte (e.g. 32-bit, 64-bit)
 * addition and subtraction.
 *
 * @param flags Flags bitmap to update the carry flag bit in.
 * @param result_8bit Result of the arithmetic operation as a
 */
void emu_reg_update_carry_flag_8bit(uint16_t* flags, uint16_t result_16bit);
void emu_reg_update_carry_flag_16bit(uint16_t* flags, uint16_t result_32bit);

void emu_reg_update_auxiliary_carry_flag(uint16_t* flags, uint16_t left, uint16_t right, uint16_t result);

/**
 * Updates the sign flag in the flags register.
 *
 */
void emu_reg_update_sign_flag_8bit(uint16_t* flags, uint8_t result);
void emu_reg_update_sign_flag_16bit(uint16_t* flags, uint16_t result);

/**
 * Updates the zero flag in the flags register.
 * Zero Flag Mask, used to isolate the zero flag bit from the flags register.
 * ZF (zero flag): If the result of an arithmetic or logical operation is zero,
 * then ZF is set; otherwise ZF is cleared. A conditional jump instruction can
 * be used to alter the flow of the program if the result is or is not zero.
 *
 * @param flags Flags bitmap to update the parity flag bit in.
 * @param arithmetic_result Result of the arithmetic operation which controls
 *  whether to set the flag bit high or low.
 */
static inline void emu_reg_update_zero_flag(uint16_t* flags, uint16_t arithmetic_result) {
    if (arithmetic_result == 0) {
        emu_reg_set_flag(flags, FLAG_ZF_MASK);
    } else {
        emu_reg_clear_flag(flags, FLAG_ZF_MASK);
    }
}

/**
 * Updates the parity flag in the flags register.
 * PF (parity flag): If the low-order eight bits of an arithmetic or logical
 * result contain an even number of 1-bits, then the parity flag is set;
 * otherwise it is cleared. PF is provided for 8080/8085 compatibility; it also
 * can be used to check ASCII characters for correct parity.
 *
 * @param flags Flags bitmap to update the parity flag bit in.
 * @param arithmetic_result Result of the arithmetic operation which controls
 *  whether to set the parity bit high or low.
 */
void emu_reg_update_parity_flag(uint16_t* flags, uint16_t arithmetic_result);

/**
 * Updates the overflow flag in the flags register. Indicates the result isn't
 * mathematically correct when interpreted as signed.
 *
 * OF (overflow flag): If the result of an operation is too large a posiive number, or too
 * small a negative number to fit in the destination operand (excluding the sign bit),
 * then OF is set; otherwise OF is cleared. OF thus indicates signed arithmetic overflow;
 * it can be tested with a conditional jump or the INTO (interrupt on overflow) instruction.
 * OF may be ignored when performing unsigned arithmetic.
 *
 * @param flags Flags bitmap to updaet the overflow flag bit in.
 * @param left Left operand
 * @param right Right operand
 * @param arithmetic_result Result of the operation performed,
 *
 */
void emu_reg_update_overflow_flag_8bit(uint16_t* flags, uint8_t left, uint8_t right, uint8_t arithmetic_result);
void emu_reg_update_overflow_flag_16bit(uint16_t* flags, uint16_t left, uint16_t right, uint16_t arithmetic_result);

#endif // EMU_REGISTERS
