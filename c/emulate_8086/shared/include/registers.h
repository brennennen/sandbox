/**
 *
 *
 */
#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>

// https://stackoverflow.com/questions/9130349/how-many-registers-are-there-in-8086-8088
typedef struct {
    uint16_t ax;
    uint16_t bx;
    uint16_t cx;
    uint16_t dx;
    uint16_t si;
    uint16_t di;
    uint16_t bp;
    uint16_t sp;

    // segment registers
    uint16_t cs;
    uint16_t ds;
    uint16_t es;
    uint16_t ss;

    // special purpose
    uint16_t ip;

    // flag register
    // 9 flags, need 2 bytes at a minimum
    // TODO: 1 byte per flag? or create a bitmap? or just another uint16_t?

} registers_t;



#endif // REGISTERS_H
