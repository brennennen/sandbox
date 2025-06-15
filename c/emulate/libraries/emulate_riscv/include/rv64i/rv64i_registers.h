
#ifndef REGISTERS_RV64I_H
#define REGISTERS_RV64I_H

#include <stdint.h>

typedef struct {
    uint64_t regs[32];
    uint32_t pc;
} registers_rv64i_t;

// TODO: provide accessor functions for aliased names (args = x0, x1, etc. temp/local
// vars = t0, t1, etc.)


#endif // REGISTERS_RV64I_H
