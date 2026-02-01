/**
 * x64:
 *
 * i386:
 * XCHG (Exchange) swaps the contents of two operands. This instruction takes
 * the place of three MOV instructions. It does not require a temporary
 * location to save the contents of one operand while load the other is being
 * loaded. XCHG is especially useful for implementing semaphores or similar
 * data structures for process synchronization (Intel 80386 Programmer's Reference
 * Manual, 3.1.2, page 46).
 *
 * XCHG exchanges two operands. The operands can be in either order. If a
 * memory operand is involved, BUS LOCK is asserted for the duration of the
 * exchange, regardless of the presence or absence of the LOCK prefix or of the
 * value of the IOPL (Intel 80386 Programmer's Reference Manual, 17.2.2.11,
 * page 409).
 *
 * 8086:
 * `XCHG destination,source`
 * XCHG (exchange) switches the contents of the source and destination (byte
 * or word) operands. When used in conjunction with the LOCK prefix, XCHG can
 * test and set a semaphore that controls access toa resource shared by multiple
 * processors.
 *
 * This instruction can be mapped into 2 different opcode encodings. The order
 * of these encodings will follow the datasheet table 4-12 (8086 Family Users
 * Manual, page 4-22, pdf page ~165).
 *
 * 1. I_EXCHANGE
 * 2. I_EXCHANGE_AX
 *
 */
#ifndef EMU_XCHG_H
#define EMU_XCHG_H

#include <stdint.h>

#include "8086/instruction_tags_8086.h"

#include "libraries/emulate_intel/include/emulate.h"

// MARK: 1. I_EXCHANGE
emu_result_t decode_exchange(
    emulator_8086_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);
emu_result_t emu_exchange(emulator_8086_t* emulator, uint8_t byte1);

// MARK: 2. I_EXCHANGE_AX
emu_result_t decode_exchange_ax(
    emulator_8086_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);
emu_result_t emu_exchange_ax(emulator_8086_t* emulator, uint8_t byte1);

#endif  // EMU_XCHG_H
