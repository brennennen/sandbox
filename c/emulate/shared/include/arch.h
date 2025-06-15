
#ifndef ARCH_H
#define ARCH_H

#include <string.h>

typedef enum {
    ARCH_INVALID = -1,
    ARCH_I8086,
    ARCH_I386,
    ARCH_RV64I,
    ARCH_COUNT
} arch_t;

static char arch_names[][16] = {
    "i8086",
    "i386",
    "rv64i"
};

static inline arch_t arch_get_by_name(char* name) {
    for (int i = 0; i < ARCH_COUNT; i++) {
        if (strncmp(name, arch_names[i], sizeof(arch_names[i])) == 0) {
            return(i);
        }
    }
    return(-1);
}

#endif // ARCH_H
