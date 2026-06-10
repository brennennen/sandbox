#pragma once
#include <stdbool.h>

typedef struct {
    bool is_open;
    int  scroll_offset;
    char input_buffer[256];
} debug_console_t;

void debug_console_init(debug_console_t* console);
void debug_console_toggle(debug_console_t* console);
void debug_console_draw(debug_console_t* console);
