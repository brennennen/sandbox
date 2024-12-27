/**
 * gcc ./padding.c && ./a.out
 * 
 * Small exploration around struct padding in c. 
 * 
 * 4 Rules to struct padding:
 * 
 * 1. Pad primitives to addresses relative to their size, that is: short to 2, int to 4,
 * long to 8.
 * 2. Don't pad inside arrays.
 * 3. Structs inside of structs pad to the inner structs largest primititive (short to 2,
 * int to 4, etc).
 * 4. Pad at the end of the struct so it can be used in an array.
 * 
 * Further Reading:
 *  * https://stackoverflow.com/questions/4306186/structure-padding-and-packing
 *  * http://www.catb.org/esr/structure-packing/
 * 
 * TODO: look into c11 stdalign.h
 */

#include <stdint.h>
#include <stdio.h>

// Disables padding, reducing the number of bytes in a struct, but requires additional
// instructions to access. It might be advantageous to disable padding to keep data.
//#define PACK_MESSAGES 1
#ifdef PACK_MESSAGES
#define PACK_ATTRIBUTE __attribute__((__packed__))
#else
#define PACK_ATTRIBUTE
#endif

// Short message struct example.
// Example: 0x  01 00 02 00
// rule 1 - field_2 is aligned to address 2 because it's divisible by its size (2). 1 byte padding added before to go from 1 to 2.
typedef struct PACK_ATTRIBUTE {     // Address      Size        Data
    uint8_t field_1;                // 0            1           01
    // padding: 1 byte              // 1            1           00
    uint16_t field_2;               // 2            2           02 00
} my_short_message_t;               // size: 4 (3 packed)

// Medium message struct example.
// Example: 0x  01 00 02 00 03 00 00 00 04 00
//              00 00
// rule 1 - field_2 is aligned to address 2 because it's divisible by its size (2). 1 byte padding added before to go from 1 to 2.
// rule 1 - field_3 is aligned to address 8 because it's divisible by its size (4). 2 bytes of padding added before to go from 6 to 8.
typedef struct PACK_ATTRIBUTE {     // Address      Size        Data
    uint8_t field_1;                // 0            1           01
    // padding: 1 byte              // 1            1           00
    uint16_t field_2[2];            // 2            4 (2 * 2)   02 00 03 00
    // padding: 2 bytes             // 6            2           00 00
    uint32_t field_3;               // 8            4           04 00 00 00
} my_medium_message_t;              // size: 12 (9 packed)

// Long message struct example.
// Example: 0x  01 00 00 00 02 00 00 00 03 00
//              03 00 04 00 00 00 05 06 00 00
//              00 00 00 00 07 00 00 00 00 00
//              00 00 08 00 08 00 08 00 08 00
//              09 00 00 00 0a 00 00 00
// rule 1 - field_2 is aligned to address 4 because it's divisible by its size (4). 3 bytes of padding added before to go from 1 to 4.
// rule 1 - field_7 is aligned to address 24 because it's divisible by its size (8). 8 bytes of padding added before to go from 18 to 24.
// rule 4 - end padding is added of the whole struct in an array.
typedef struct PACK_ATTRIBUTE {     // Address      Size        Data
    uint8_t field_1;                // 0            1           01
    // padding: 3 bytes             // 1            3           00 00 00
    uint32_t field_2;               // 4            4           02 00 00 00
    uint16_t field_3[2];            // 8            4 (2 * 2)   03 00 03 00
    uint32_t field_4;               // 12           4           04 00 00 00
    uint8_t field_5;                // 16           1           05
    uint8_t field_6;                // 17           1           06
    // padding: 6 byte              // 18           6           00 00 00 00 00 00
    uint64_t field_7;               // 24           8           07 00 00 00 00 00 00 00
    uint16_t field_8[4];            // 31           8 (2 * 4)   08 00 08 00 08 00 08 00
    uint32_t field_9;               // 40           4           09 00 00 00
    uint8_t field_10;               // 44           1           0a
    // padding: 3 byte              // 45           3           00 00 00
} my_long_message_t;                // size: 48 (36 packed)


// Example: 0x  01 70 0e 7b 02 00 00 00 03 00
//              03 00 00 00 00 00 01 00 00 00
//              02 00 00 00 03 00 03 00 04 00
//              00 00 05 06 00 00 00 00 00 00
//              07 00 00 00 00 00 00 00 08 00
//              08 00 08 00 08 00 09 00 00 00
//              0a 00 00 00 05 00 00 00 00 00
//              00 00 06 00 00 00 00 00 00 00
// rule 1 - field_2 is aligned to address 4 because it's divisible by its size (4). 3 bytes of padding added before to go from 1 to 4.
// rule 3 - field_4 is aligned to address 16 because it's divisible by its largest primitive size (8). 4 bytes of padding added before to go from 12 to 16.
// rule 1 - field_6 is aligned to address 72 because it's divisible by its size (8). 4 bytes of padding added before to go from 68 to 72.
typedef struct PACK_ATTRIBUTE {     // Address      Size        Data
    uint8_t field_1;                // 0            1           01
    // padding: 3 bytes             // 1            3           70 0e 7b
    uint32_t field_2;               // 4            4           02 00 00 00
    uint16_t field_3[2];            // 8            4           03 00 03 00
    // padding: 4 bytes             // 12           4           00 00 00 00
    my_long_message_t field_4;      // 16           48          01 00 00 00 02 00 00 00 ...
    uint32_t field_5;               // 64           4           05 00 00 00
    // padding: 4 bytes             // 68           4           00 00 00 00
    uint64_t field_6;               // 72           8           06 00 00 00 00 00 00 00
} my_ultra_long_message_t;          // size: 80 (57 packed)

static void debug_print_struct(void* object, unsigned long struct_size) {
    printf("object size: %ld\n", struct_size);
    printf("object data: 0x ");
    for(int i = 0; i < struct_size; i++) {
        printf("%.2x ", ((uint8_t*)object)[i]);
    }
    printf("\n");
}

int main (int argc, char *argv[]) {
    my_short_message_t my_short_message = {.field_1 = 1, .field_2 = 2};
    debug_print_struct(&my_short_message, sizeof(my_short_message));

    my_medium_message_t my_medium_message = {.field_1 = 1, .field_2[0] = 2, .field_2[1] = 3, .field_3 = 4};
    debug_print_struct(&my_medium_message, sizeof(my_medium_message));

    my_long_message_t my_long_message = {
        .field_1 = 1,
        .field_2 = 2,
        .field_3[0] = 3,
        .field_3[1] = 3,
        .field_4 = 4,
        .field_5 = 5,
        .field_6 = 6,
        .field_7 = 7,
        .field_8[0] = 8,
        .field_8[1] = 8,
        .field_8[2] = 8,
        .field_8[3] = 8,
        .field_9 = 9,
        .field_10 = 10
    };
    debug_print_struct(&my_long_message, sizeof(my_long_message));

    my_ultra_long_message_t my_ultra_long_message = {
        .field_1 = 1,
        .field_2 = 2,
        .field_3[0] = 3,
        .field_3[1] = 3,
        .field_4 = my_long_message,
        .field_5 = 5,
        .field_6 = 6
    };
    debug_print_struct(&my_ultra_long_message, sizeof(my_ultra_long_message));
}
