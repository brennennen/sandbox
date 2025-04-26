/**
 * `CLC`
 * CLC (Clear Carry flag) zeroes the carry flag (CF) and affects no other flags. It (and
 * CMC and STC) is useful in conjunction with the TCL and RCR instructions (8086 Family
 * Users Manual, page 2-47, pdf page ~62).
 *
 * CLC has 1 machine instruction on 8086. See datasheet table 4-12 (8086 Family Users Manual,
 * page 4-27, pdf page ~169).
 *
 */

#ifndef EMU_8086_CLC_H
#define EMU_8086_CLC_H

#include <stdint.h>

#include "8086/emulate_8086.h"

// MARK: CLC
emu_result_t decode_clc(
    emulator_8086_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);
emu_result_t emu_clc(emulator_8086_t* emulator, uint8_t byte1);

#endif // EMU_8086_CLC_H
