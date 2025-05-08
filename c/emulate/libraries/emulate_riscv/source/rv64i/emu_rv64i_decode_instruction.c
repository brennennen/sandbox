


#include <stdint.h>

#include "logger.h"

#include "rv64i/instructions_rv64i.h"
#include "rv64i/emu_rv64i_decode_instruction.h"

instruction_tag_rv64i_t emu_rv64i_decode_instruction_tag(uint32_t instruction) {
    printf("emu_rv64i_decode_instruction_tag: data: 0x%04x\n", instruction);

    // opcode is entirely within the least significant 8 bits?
    uint8_t opcode = instruction & 0b01111111;
    printf("opcode: %x\n", opcode);

    switch(opcode) {
        // ...
        case(0b0010011): {
            return(I_RV64I_ADD_IMMEDIATE);
        }
        // ...
    }

    return(I_RV64I_INVALID);
}
