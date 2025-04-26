#include <string.h>
#include "i386/emulate_i386.h"


result_t emu_i386_init(emulator_i386_t* emulator) {
    emulator->stack_size = STACK_SIZE; // using a size here in case i want to make this dynamic/resizable later.
    emulator->stack_top = 0;
    emulator->memory_size = MEMORY_SIZE;
    memset(emulator->stack, 0, emulator->stack_size);
}
