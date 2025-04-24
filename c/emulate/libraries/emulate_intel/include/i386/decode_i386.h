/**
 * Decodes machine code for 8086 assembly per the "8086 family users manual 1".
 */
#ifndef DECODE_i386_H
#define DECODE_i386_H

#include <stdio.h>

#include "shared/include/result.h"
#include "shared/include/instructions.h"
#include "shared/include/registers.h"

#include "emulate.h"

result_t emu_i386_decode(emulator_t* decoder, char* out_buffer, size_t out_buffer_size);

#endif // DECODE_i386_H
