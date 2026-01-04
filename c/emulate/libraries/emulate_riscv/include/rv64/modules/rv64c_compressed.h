
#ifndef EMU_RV64A_COMPRESSED_H
#define EMU_RV64A_COMPRESSED_H

#include <stdio.h>
#include <stdint.h>

#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"

uint32_t rv64c_expand(uint16_t compressed);

#endif // EMU_RV64A_COMPRESSED_H
