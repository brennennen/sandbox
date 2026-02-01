
#ifndef SYSBUS_H
#define SYSBUS_H

#include <stdint.h>

typedef struct device device_t;

typedef uint64_t (*device_read_fn)(void* ctx, uint64_t offset, int size);
typedef uint64_t (*device_write_fn)(void* ctx, uint64_t offset, int size);

typedef struct device {
    const char* name;
    uint64_t base_address;
    uint64_t size;
    void* context;
    device_read_fn read;
    device_write_fn write;
} device_t;

void bus_register_device(device_t* device);
int bus_read(uint64_t physical_address, int size, uint64_t* out_value);
int bus_write(uint64_t physical_address, int size, uint64_t value);

#endif  // SYSBUS_H
