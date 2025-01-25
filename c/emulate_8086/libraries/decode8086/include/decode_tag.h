#ifndef DECODE_TAG_H
#define DECODE_TAG_H

#include <stdint.h>

#include "shared/include/instructions.h"

instruction_tag_t dcd_decode_tag(uint8_t byte1, uint8_t byte2);

#endif // DECODE_TAG_H

