
#ifndef EMU_RV64A_ATOMIC_H
#define EMU_RV64A_ATOMIC_H

#include <stdint.h>

#include "rv64/rv64_hart.h"
#include "rv64/rv64_instructions.h"
#include "shared/include/result.h"

emu_result_t rv64a_atomic_emulate(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
);

#endif  // EMU_RV64A_ATOMIC_H
