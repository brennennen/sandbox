
#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"

#include "libraries/decode8086/include/decode8086.h"
#include "libraries/decode8086/include/decode_utils.h"
#include "libraries/decode8086/include/decode_shared.h"

#include "libraries/decode8086/include/instructions/decode_conditional_jumps.h"

// Naming this "decode_conditional_jump" causes linker errors with criterion's unit test system
// TODO: find a better name that works or change unit test libraries if more issues pop up.
decode_result_t decode_conditional_jump2(
    decoder_t* decoder,
    instruction_tag_t tag,
    uint8_t byte1,
    conditional_jump_t* conditional_jump
) {
    conditional_jump->fields1 = byte1;
    decode_result_t read_byte2_result = dcd_read_byte(decoder, (uint8_t*) &conditional_jump->jump_offset);
    return read_byte2_result;
}

void write_conditional_jump(
    conditional_jump_t* conditional_jump,
    instruction_tag_t tag,
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
    int offset = conditional_jump->jump_offset + 2;
    int written = 0;
    if (offset >= 0) {
        written = snprintf(buffer + *index, buffer_size - *index, "%s $+%d",
                           mnemonic,
                           conditional_jump->jump_offset + 2);
    } else {
        written = snprintf(buffer + *index, buffer_size - *index, "%s $%d",
                           mnemonic,
                           conditional_jump->jump_offset + 2);
    }
    if (written < 0) {
        // TODO: propogate error
    }
    *index += written;
}
