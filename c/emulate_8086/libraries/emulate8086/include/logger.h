#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#define DO_LOG 1
#define DO_DEBUG_LOG 1

/**
 * Standard log levels. Tightly coupled with `log_level_names` static string array.
 */
typedef enum {
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
} log_level_t;

/**
 * Log levels as seen in the log file. Tightly coupled with `log_level_t` enum.
 */
static const char* log_level_names[] = {
    "INFO ", "WARN ", "ERROR"
};

/**
 * Maximum size of the string buffer used to hold the log message.
 */
#define MAX_LOG_MESSAGE_LENGTH 4096

/**
 * Logs a normal log message. Consider using `LOG(...)` macro for brevity/consistency.
 * [2025-04-21T01:00:56][INFO ] hit 0x00, assuming this is end of program. ip: 260
 */
void log_message(log_level_t level, const char* format, ...);

/**
 * Logs a block of memory. Consider using `LOGMEM(...)` macro for brevity/consistency.
 * TODO: consider adding ascii representation to the right ................
 *
 * ex:
 * [time][MEM  ] message decribing the memory being dumped (hopefully)
 * 000100   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
 * 000110   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
 * 000120   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
 * 000130   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
 */
void log_memory(char* data, int data_start, int data_size, const char* format, ...);

#ifdef DO_LOG
    /**
     * Logs a normal log message.
     * [2025-04-21T01:00:56][INFO ] hit 0x00, assuming this is end of program. ip: 260
     */
    #define LOG(level, format, ...) log_message(level, format, ##__VA_ARGS__)

    /**
     * Logs a block of memory.
     * TODO: consider adding ascii representation to the right ................
     *
     * ex:
     * [time][MEM  ] message decribing the memory being dumped (hopefully)
     * 000100   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
     * 000110   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
     * 000120   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
     * 000130   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
     */
    #define LOGMEM(data, start, size, format, ...) log_memory(data, start, size, format, ##__VA_ARGS__)
#else
    #define LOG(level, format, ...)
    #define LOGMEM(level, data, start, size, format, ...)
#endif

/**
 * Logs a debug message that can easily by compiled out. Consider using
 * macro: `LOGD(...)` for brevity.
 */
void log_debug_message(const char* func, const char* format, ...);

/**
 * Logs a debug assembly instruction message that can easily by compiled out.
 * Consider using macro: `LOGDIW(...)` for brevity.
 */
void log_debug_instruction(const char* func, const char* format, ...);

#ifdef DEBUG
    #ifdef DO_DEBUG_LOG
        /**
         * Logs a debug message that can easily be compiled out to improve performance.
         * Example output:
         * `[2025-04-21T01:00:56][DEBUG][emu_next] hit 0x00, assuming this is end of program. ip: 258`
         */
        #define LOGD(format, ...) log_debug_message(__func__, format, ##__VA_ARGS__)
        /**
         * Logs an instruction message (no standard leading sections, ex: `[i] message` instead of
         * `[time][level][func] message`).
         * Example output: `[i] add cx, 12`
         */
        #define LOGDI(format, ...) log_debug_instruction(__func__, format, ##__VA_ARGS__)
        /**
         * Log Debug Instruction (with) Write (function)
         * Usage: `LOGDIW(write_not, direction, wide, mod, reg, rm, displacement);`
         * When to use: In each emulated instruction to provide a trace of what instructions were executed.
         * TODO: consider renaming to something with "trace" in the name
         */
        #define LOGDIW(write_func, ...) \
            do { \
                int logdiw_index = 0; \
                char logdiw_buffer[64]; \
                write_func(__VA_ARGS__, logdiw_buffer, &logdiw_index, sizeof(logdiw_buffer)); \
                LOGDI("%s", logdiw_buffer); \
            } while (0)
    #else
        #define LOGD(format, ...)
        #define LOGDI(format, ...)
        #define LOGDI2(write_func, ...)
    #endif
#endif

#endif
