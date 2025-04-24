#ifndef DECODE_i386_TAG_H
#define DECODE_i386_TAG_H

#include <stdint.h>

#include "i386/i386_instruction_tags.h"

i386_instruction_tag_t emu_i386_decode_tag(uint8_t byte1, uint8_t byte2);

#endif // DECODE_i386_TAG_H

