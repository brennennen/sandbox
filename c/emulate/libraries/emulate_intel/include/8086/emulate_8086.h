/**
 * Emulator for 8086 assembly per the "8086 family users manual 1".
 */
#ifndef EMULATE_8086_H
#define EMULATE_8086_H

#include <stdio.h>

#include "shared/include/result.h"
#include "shared/include/instructions.h"
#include "shared/include/registers.h"

#include "libraries/emulate_intel/include/emulate.h"

result_t emu_8086_emulate(emulator_t* emulator);

#endif // EMULATE_8086_H
