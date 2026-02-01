
#ifndef EMU_I386_UTILS
#define EMU_I386_UTILS

#include "shared/include/result.h"

#include "i386/emulate_i386.h"

emu_result_t emu_i386_read_m8(emulator_i386_t* emulator, uint8_t* out_byte);
emu_result_t emu_i386_read_m16(emulator_i386_t* emulator, uint16_t* out_word);

#endif  // EMU_I386_UTILS
