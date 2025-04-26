#ifndef DECODE_8086_TAG_H
#define DECODE_8086_TAG_H

#include <stdint.h>

#include "8086/instruction_tags_8086.h"

instruction_tag_8086_t emu_8086_decode_tag(uint8_t byte1, uint8_t byte2);

#endif // DECODE_8086_TAG_H

