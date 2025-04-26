#ifndef DECODE_8086_UTILS_H
#define DECODE_8086_UTILS_H

#include "shared/include/binary_utilities.h"
#include "shared/include/result.h"

#include "emulate.h"


emu_result_t dcd_read_byte(emulator_8086_t* emulator, uint8_t* out_byte);
emu_result_t dcd_read_word(emulator_8086_t* emulator, uint16_t* out_word);


#endif // DECODE_8086_UTILS_H