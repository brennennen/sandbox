
#include "debug_console.h"
#include "logger.h"

void debug_console_draw(debug_console_t* console) {
    if (!console->is_open)
        return;

    // todo: draw the background panel

    // todo: fetch and draw the logs
    int total_logs = logger_get_history_count();

    for (int i = 0; i < total_logs; i++) {
        const char* text = logger_get_history_line(i);
        // todo: Draw 'text' to the screen at the correct Y-offset based on console->scroll_offset
    }

    // todo: draw the input buffer at the bottom
}
