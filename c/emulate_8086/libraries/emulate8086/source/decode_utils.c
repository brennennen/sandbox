
#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"

#include "libraries/emulate8086/include/emulate8086.h"

// TODO: move these back to emulate8086 after they work with emulator->memory

emu_result_t dcd_read_byte(emulator_t* emulator, uint8_t* out_byte) {
    if (emulator->registers.ip >= emulator->memory_size) {
        printf("dcd_read_byte: ER_OUT_OF_BOUNDS. ip (+ read size): %d >= memory size: %d\n",
            emulator->registers.ip, emulator->memory_size);
        return ER_OUT_OF_BOUNDS;
    }
    *out_byte = emulator->memory[emulator->registers.ip];
    emulator->registers.ip += 1;
    return ER_SUCCESS;
}

emu_result_t dcd_read_word(emulator_t* emulator, uint16_t* out_word) {
    if (emulator->registers.ip + 1 >= emulator->memory_size) {
        printf("dcd_read_word: ER_OUT_OF_BOUNDS. ip (+ read size): (%d + 1) >= memory size: %d\n",
            emulator->registers.ip, emulator->memory_size);
        return ER_OUT_OF_BOUNDS;
    }
    *out_word = emulator->memory[emulator->registers.ip] | (emulator->memory[emulator->registers.ip + 1] << 8);
    emulator->registers.ip += 2;
    return ER_SUCCESS;
}
