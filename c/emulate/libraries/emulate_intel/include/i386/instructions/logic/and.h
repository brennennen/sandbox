/**
 * i386: (not implemented)
 * around (Intel 80386 Programmer's Reference Manual, 3.1.2, page 46).
 *
 * AND - Logical AND
 * | opcode   | instruction        | clocks | description                                     |
 * |----------|--------------------|--------|-------------------------------------------------|
 * | 24 ib    | AND AL, imm8       | 2      | AND immediate byte to AL                        |
 * | 25 iw    | AND AX, imm16      | 2      | AND immediate word to AX                        |
 * | 25 id    | AND EAX, imm32     | 2      | AND immediate dword to EAX                      |
 * | 80 /4 ib | AND r/m8, imm8     | 2/7    | AND immediate byte to r/m byte                  |
 * | 81 /4 iw | AND r/m16, imm16   | 2/7    | AND immediate byte to r/m word                  |
 * | 81 /4 id | AND r/m32, imm32   | 2/7    | AND immediate byte to r/m dword                 |
 * | 83 /4 ib | AND r/m16, imm8    | 2/7    | AND sign-extended immediate byte with r/m word  |
 * | 83 /4 ib | AND r/m32, imm8    | 2/7    | AND sign-extended immediate byte with r/m dword |
 * | 20 /r    | AND r/m8, r8       | 2/7    | AND byte register to r/m byte                   |
 * | 21 /r    | AND r/m16, r16     | 2/7    | AND word register to r/m word                   |
 * | 21 /r    | AND r/m32, r32     | 2/7    | AND dword register to r/m dword                 |
 * | 22 /r    | AND r8, r/m8       | 2/7    | AND r/m byte to byte register                   |
 * | 23 /r    | AND r16, r/m16     | 2/7    | AND r/m word to word register                   |
 * | 23 /r    | AND r32, r/m32     | 2/7    | AND r/m dword to dword register                 |
 *
 * Operation
 * DEST <- DEST AND SRC;
 * CF <- 0;
 * OF <- 0;
 *
 * Description
 * Each bit of the result of the AND instruction is a 1 if both corresponding bits
 * of the operands are 1; otherwise, it becomes a 0.
 *
 * Flags Affected
 * CF = 0, OF = 0, PF, SF, and ZF as described in Appendix C (TODO: describe this here?)
 *
 * Protected Mode Exceptions
 * TODO: understand this better before plopping it here
 *
 * Real Address Mode Exceptions
 * TODO: understand this better before plopping it here
 *
 * Virtual 8086 Mode Exceptions
 * TODO: understand this better before plopping it here
 *
 * (Intel 80386 Programmer's Reference Manual, 17.2.2.11, page 262).
 */
#ifndef EMU_i386_AND_H
#define EMU_i386_AND_H

#include <stdint.h>

#include "emulate.h"

// MARK: I_i386_AND
emu_result_t emu_i386_decode_and(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);

#endif // EMU_i386_AND_H
