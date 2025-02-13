
#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"

#include "libraries/emulate8086/include/emulate8086.h"
#include "libraries/emulate8086/include/decode_utils.h"
#include "libraries/emulate8086/include/decode_shared.h"

#include "libraries/emulate8086/include/instructions/conditional_jumps.h"

// Naming this "decode_conditional_jump" causes linker errors with criterion's unit test system
// TODO: find a better name that works or change unit test libraries if more issues pop up.
emu_result_t decode_conditional_jump2(
    emulator_t* emulator,
    instruction_tag_t tag,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
) {
    int8_t jump_offset = 0;
    emu_result_t result = dcd_read_byte(emulator, (uint8_t*) &jump_offset);
    write_conditional_jump(tag, jump_offset, out_buffer, index, out_buffer_size);
    return result;
}

emu_result_t emu_conditional_jump(emulator_t* emulator, instruction_tag_t tag,  uint8_t byte1) {
    int8_t jump_offset = 0;
    emu_result_t result = dcd_read_byte(emulator, (uint8_t*) &jump_offset);

    // TODO

    return ER_FAILURE;
}


void write_conditional_jump(
    instruction_tag_t tag,
    int8_t jump_offset,
    char* buffer,
    int* index,
    int buffer_size
) {

    //char* mnemonic = "je";
    char* mnemonic = instruction_tag_mnemonic[tag];
    // offset is measured in number of bytes from next address. so for:
    // label:
    // je label
    // this would be the same as "je $+0". but the IP-INC8 will be -2.
    // "je $-2" would have an IP-INC8 of -4.
    // so we need to add +2 to IP-INC8 to re-create the relative offsets.
    //
    int offset = jump_offset + 2;
    int written = 0;
    if (offset >= 0) {
        written = snprintf(buffer + *index, buffer_size - *index, "%s $+%d",
                           mnemonic,
                           jump_offset + 2);
    } else {
        written = snprintf(buffer + *index, buffer_size - *index, "%s $%d",
                           mnemonic,
                           jump_offset + 2);
    }
    if (written < 0) {
        // TODO: propogate error
    }
    *index += written;
    snprintf(buffer + *index, buffer_size - *index, "\n");
    *index += 1;
}
