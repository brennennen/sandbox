
#ifndef BUS_H
#define BUS_H

#include "shared/include/sysbus.h"

#define MAX_DEVICES 16
static device_t* device_map[MAX_DEVICES];
static int device_count;

void bus_register_device(device_t* device) {
    if (device_count < MAX_DEVICES) {
        device_map[device_count++] = device;
    }
}

int bus_write(uint64_t physical_address, int size, uint64_t value) {
    for (int i = 0; i < device_count; i++) {
        device_t* device = device_map[i];
        if (physical_address >= device->base_address
            && physical_address < (device->base_address + device->size)) {
        }
    }
}

#endif  // BUS_H
