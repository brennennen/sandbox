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
 * 8086: (partially implemented)
 * `NOT destination`
 * NOT inverts the bits (forms the one's complement) of the boyte or word
 * operand (8086 Family Users Manual, page 2-38, pdf page ~53).
 *
 * NOT (invert) has 1 machine instruction on 8086. See datasheet table 4-12
 * (8086 Family Users Manual, page 4-24, pdf page ~166).
 */

#ifndef EMU_NOT_H
#define EMU_NOT_H

#include <stdint.h>

#include "shared/include/instructions.h"

#include "libraries/emulate8086/include/emulate8086.h"

// MARK: NOT
emu_result_t decode_not(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);
emu_result_t emu_not(emulator_t* emulator, uint8_t byte1);

#endif // EMU_NOT_H
