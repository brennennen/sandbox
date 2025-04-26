#ifndef DECODE_i386_TAG_H
#define DECODE_i386_TAG_H

#include <stdint.h>

#include "i386/instruction_tags_i386.h"

instruction_tag_i386_t emu_i386_decode_tag(uint8_t byte1, uint8_t byte2);

#endif // DECODE_i386_TAG_H

