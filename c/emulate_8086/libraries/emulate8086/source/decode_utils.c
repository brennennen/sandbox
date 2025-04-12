
#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"

#include "libraries/emulate8086/include/emulate8086.h"

emu_result_t dcd_read_byte(emulator_t* emulator, uint8_t* out_byte) {
    if (emulator->program_buffer_index >= emulator->program_buffer_size) {
        printf("dcd_read_byte: ER_OUT_OF_BOUNDS. i: %d >= s: %ld\n",
            emulator->program_buffer_index, emulator->program_buffer_size);
        return ER_OUT_OF_BOUNDS;
    }
    *out_byte = emulator->program_buffer[emulator->program_buffer_index];
    emulator->program_buffer_index += 1;
    return ER_SUCCESS;
}

emu_result_t dcd_read_word(emulator_t* emulator, uint16_t* out_word) {
    if (emulator->program_buffer_index + 1 >= emulator->program_buffer_size) {
        printf("dcd_read_word: ER_OUT_OF_BOUNDS. i: %d >= s: %ld\n",
            emulator->program_buffer_index, emulator->program_buffer_size);
        return ER_OUT_OF_BOUNDS;
    }
    *out_word = emulator->program_buffer[emulator->program_buffer_index] | (emulator->program_buffer[emulator->program_buffer_index + 1] << 8);
    emulator->program_buffer_index += 2;
    return ER_SUCCESS;
}
