
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "sysbus.h"

typedef struct {
    uint8_t thr;
    uint8_t lsr;

} uart_8250_state_t;

static uint64_t uart_read(void* context, uint64_t offset, int size) {
    uart_8250_state_t* uart = (uart_8250_state_t*)context;
    if (offset == 5) {
        return 0x20 | 0x40;  // Transmitter Empty | Holding Register Empty
    }
    return 0;
}

static uint64_t uart_write(void* context, uint64_t offset, uint64_t value, int size) {
    uart_8250_state_t* uart = (uart_8250_state_t*)context;
    if (offset == 0) {  // THR: Transmit
        putchar((char)value);
        fflush(stdout);
    }
    return 0;
}

device_t* create_uart_8250(uint64_t base_address) {
    uart_8250_state_t* state = calloc(1, sizeof(uart_8250_state_t));
    device_t* device = calloc(1, sizeof(device_t));
    device->name = "ns16550a";
    device->base_address = base_address;
    device->context = state;
    device->read = uart_read;
    device->write = uart_write;
}
