#ifndef RV64_SHARED_SYSTEM_H
#define RV64_SHARED_SYSTEM_H

#include <stdint.h>



typedef struct rv64_shared_system_s {
    int memory_size;
    uint8_t memory[4096];
} rv64_shared_system_t;



#endif // RV64_SHARED_SYSTEM_H
