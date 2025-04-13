/**
 * x64: (not implemented)
 *
 * i386: (not implemented)
 * NOT (Not) inverts the bits in the specified operand to form a one's
 * complement of the operand. The NOT instruction is a unary operation that
 * uses a single operand in a register or memory. NOT has no effect on the
 * flags (Intel 80386 Programmer's Reference Manual, 3.1.2, page 46).
 *
 * NOT - One's Complement Negation
 * | opcode | instruction | clocks | description                    |
 * |--------|-------------|--------|--------------------------------|
 * | F6 /2  | NOT r/m8    | 2/6    | Reverse each bit of r/m byte   |
 * | F7 /2  | NOT r/m16   | 2/6    | Reverse each bit of r/m word   |
 * | F7 /2  | NOT r/m32   | 2/6    | Reverse each bit of r/m dword  |
 * Operation:
 *      r/m <- NOT r/m
 * Description:
 *      NOT inverts the operand; every 1 becomes a 0, and vice versa.
 * Flags Affected:
 *      None
 * Protected Mode Exceptions:
 *      #GP(0) if the result is in a nonwritable segment; #GP(0) for an
 *      illegal memory operand effective address in the CS, DS, ES, FS, or
 *      GS segments; #SS(0) for an illegal address in the SS segment;
 *      #PF(fault-code) for a page fault
 * Real Address Mode Exceptions:
 *      Interrupt 13 if any part of the operand would lie outside of the
 *      effective address space from 0 to 0FFFFH
 * Virtual 8086 Mode Exceptions:
 *      Same exceptions as in real-address mode; #PF(fault-code) for a page
 *      fault.
 * (Intel 80386 Programmer's Reference Manual, 17.2.2.11, page 356).
 *
 * 8086: (fully implemented)
 * `NOT destination`
 * NOT inverts the bits (forms the one's complement) of the boyte or word
 * operand (8086 Family Users Manual, page 2-47, pdf page ~53).
 *
 * NOT (invert) has 1 machine instruction on 8086. See datasheet table 4-12
 * (8086 Family Users Manual, page 4-27, pdf page ~166).
 */

 #include <string.h>

 #include "shared/include/binary_utilities.h"
 #include "shared/include/instructions.h"
 #include "shared/include/result.h"

 #include "libraries/emulate8086/include/emulate8086.h"
 #include "libraries/emulate8086/include/emu_registers.h"
 #include "libraries/emulate8086/include/decode_utils.h"
 #include "libraries/emulate8086/include/decode_shared.h"

 #include "libraries/emulate8086/include/instructions/logic/not.h"


// MARK: NOT

// MARK: DECODE

/**
 * Decodes a `not` instruction on direct memory addresses.
 * Examples:
 * `not byte [1000]`
 * `not word [1000]`
 */
static emu_result_t decode_not__direct_access(char* out_buffer, int* index, wide_t wide, uint16_t displacement) {
    char* width_string = "";
    if (wide == WIDE_BYTE) {
        width_string = "byte";
    } else { // WIDE_WORD
        width_string = "word";
    }
    int written = sprintf(out_buffer + *index, "not %s [%u]", width_string, displacement);
    *index += written;
    if (written < 0) {
        return ER_FAILURE;
    }
    return ER_SUCCESS;
}

static emu_result_t decode_not__memory() {
    printf("decode_not__memory not implemented.\n");
    return ER_FAILURE;
}

static emu_result_t decode_not__register(char* out_buffer, int* index, wide_t wide, uint16_t rm) {
    char* register_string = "";
    if (wide == WIDE_BYTE) {
        register_string = regb_strings[rm];
    } else { // WIDE_WORD
        register_string = regw_strings[rm];
    }
    memcpy(out_buffer + *index, "not ", 4);
    *index += 4;
    memcpy(out_buffer + *index, register_string, 2);
    *index += 2;
    memcpy(out_buffer + *index, "\n", 2);
    *index += 2;
    return ER_SUCCESS;
}

/**
 * Decodes the 8086 `not` encoded byte code instruction.
 */
emu_result_t decode_not(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size)
{
    direction_t direction = 0;
    wide_t wide = 0;
    mod_t mod = 0;
    uint8_t reg = 0;
    uint8_t rm = 0;
    uint16_t displacement = 0;
    uint8_t instruction_size = 0;

    emu_result_t result = emu_decode_common_standard_format(
        emulator, byte1, &direction, &wide, &mod, &reg, &rm, &displacement, &instruction_size
    );

    if (mod == MOD_MEMORY && rm == REG_DIRECT_ACCESS) {
        return decode_not__direct_access(out_buffer, index, wide, displacement);
    }

    switch (mod) {
        case MOD_MEMORY:
        case MOD_MEMORY_8BIT_DISPLACEMENT:
        case MOD_MEMORY_16BIT_DISPLACEMENT: {
            return decode_not__memory();
        }
        case MOD_REGISTER: {
            return decode_not__register(out_buffer, index, wide, rm);
        }
    }

    return ER_FAILURE;
}

// MARK: EMULATE

/**
 * Performs one's complement negation on direct memory addresses.
 * Examples:
 * `not byte [1000]`
 * `not word [1000]`
 */
static emu_result_t emu_not__direct_access(emulator_t* emulator, wide_t wide, uint16_t displacement) {
    if (wide == WIDE_BYTE) {
        uint8_t source_data = 0;
        int res = emu_memory_get_byte(emulator, displacement, &source_data);
        if (res != ER_SUCCESS) {
            return res;
        }
        return emu_memory_set_byte(emulator, displacement, ~source_data);
    } else { // WIDE_WORD
        uint16_t source_data = 0;
        int res = emu_memory_get_uint16(emulator, displacement, &source_data);
        if (res != ER_SUCCESS) {
            return res;
        }
        return emu_memory_set_uint16(emulator, displacement, ~source_data);
    }
}

/**
 * Performs one's complement negation on effective memory addresses.
 * Examples:
 * `not byte [bx]`
 * `not word [bx]`
 */
static emu_result_t emu_not__memory(emulator_t* emulator, wide_t wide, uint8_t rm, mod_t mod, uint16_t displacement) {
    uint32_t address = emu_get_effective_address(&emulator->registers, rm, mod, displacement);
    if (wide == WIDE_BYTE) {
        uint8_t source_data = 0;
        int res = emu_memory_get_byte(emulator, address, &source_data);
        if (res != ER_SUCCESS) {
            return res;
        }
        return emu_memory_set_byte(emulator, address, ~source_data);
    } else { // WIDE_WORD
        uint16_t source_data = 0;
        int res = emu_memory_get_uint16(emulator, address, &source_data);
        if (res != ER_SUCCESS) {
            return res;
        }
        return emu_memory_set_uint16(emulator, address, ~source_data);
    }
}

/**
 * Performs one's complement negation on a register.
 * Examples:
 * `not bl`
 * `not bx`
 */
static emu_result_t emu_not__register(emulator_t* emulator, wide_t wide, uint8_t rm) {
    if (wide == WIDE_BYTE) {
        uint8_t* dest = emu_get_byte_register(&emulator->registers, rm);
        *dest = ~*dest;
        return ER_SUCCESS;
    } else { // WIDE_WORD
        uint16_t* dest = emu_get_word_register(&emulator->registers, rm);
        *dest = ~*dest;
        return ER_SUCCESS;
    }
}

/**
 * Implements the 8086 `not` instruction. Performs a one's complement negation on a register
 * or directly in memory.
 */
emu_result_t emu_not(emulator_t* emulator, uint8_t byte1) {
    direction_t direction = 0;
    wide_t wide = 0;
    mod_t mod = 0;
    uint8_t reg = 0;
    uint8_t rm = 0;
    uint16_t displacement = 0;
    uint8_t instruction_size = 0;

    emu_result_t result = emu_decode_common_standard_format(
        emulator, byte1, &direction, &wide, &mod, &reg, &rm, &displacement, &instruction_size
    );
    emulator->registers.ip += instruction_size;

    if (mod == MOD_MEMORY && rm == REG_DIRECT_ACCESS) {
        return emu_not__direct_access(emulator, wide, displacement);
    }

    switch (mod) {
        case MOD_MEMORY:
        case MOD_MEMORY_8BIT_DISPLACEMENT:
        case MOD_MEMORY_16BIT_DISPLACEMENT: {
            return emu_not__memory(emulator, wide, rm, mod, displacement);
        }
        case MOD_REGISTER: {
            return emu_not__register(emulator, wide, rm);
        }
    }
    return ER_FAILURE;
}
