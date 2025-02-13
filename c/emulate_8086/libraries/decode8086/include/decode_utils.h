#ifndef DECODE_UTILS_H
#define DECODE_UTILS_H

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"

#include "libraries/decode8086/include/decode8086.h"


emu_result_t dcd_read_byte(emulator_t* emulator, uint8_t* out_byte);
emu_result_t dcd_read_word(emulator_t* emulator, uint16_t* out_word);


#endif // DECODE_UTILS_H