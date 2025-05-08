
#ifndef INSTRUCTIONS_RV64I_H
#define INSTRUCTIONS_RV64I_H

#include <stdint.h>
#include <inttypes.h>

#include "shared/include/binary_utilities.h"

// MARK: Instructions
typedef enum ENUM_PACK_ATTRIBUTE {
    I_RV64I_INVALID,
    // ...
    I_RV64I_ADD_IMMEDIATE,
    // ...
} instruction_tag_rv64i_t;

#endif // INSTRUCTIONS_RV64I_H
