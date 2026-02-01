

#include "i386/instruction_tags_i386.h"

#include "i386/decode_i386_tag.h"

instruction_tag_i386_t emu_i386_decode_tag(uint8_t byte1, uint8_t byte2) {
    switch (byte1) {
        // ...
        // MARK: COMMON 0b100000xx
        case 0b10000000:    // 0x80
        case 0b10000001:    // 0x81
        case 0b10000010:    // 0x82
        case 0b10000011: {  // 0x83
            uint8_t sub_opcode = byte2 & 0b00111000;
            switch (sub_opcode) {
                // ...
                // AND 2
                case 0b00100000:  // 0b100 = "/4"
                    return I_i386_AND_IMMEDIATE;
                    // ...
            };
            break;
        }
        // ...
        // MARK: AND
        // AND opcodes 1 - 3 | I_i386_AND_IMMEDIATE | 0x24 - 0x25 | handled by common
        // AND opcodes 4 - 8 | I_i386_AND_AX | 0b0010010x | 0x80 - 0x83
        case 0b00100100:
        case 0b00100101:
            return I_i386_AND_AX;
        // AND opcodes 9 - 14 | I_i386_AND | 0b0010000xx | 0x20 - 0x23
        case 0b00100000:  // 0x20
        case 0b00100001:  // 0x21
        case 0b00100010:  // 0x22
        case 0b00100011:  // 0x23
            return I_i386_AND;
            // ...
    }
}
