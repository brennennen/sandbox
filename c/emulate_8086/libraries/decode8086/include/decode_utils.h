#ifndef DECODE_UTILS_H
#define DECODE_UTILS_H

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"

#include "libraries/decode8086/include/decode8086.h"


decode_result_t dcd_read_byte(decoder_t* decoder, uint8_t* out_byte);
decode_result_t dcd_read_word(decoder_t* decoder, uint16_t* out_word);


#endif // DECODE_UTILS_H