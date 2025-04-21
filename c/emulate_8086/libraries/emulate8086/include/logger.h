#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#define DO_LOG 1
#define DO_DEBUG_LOG 1

typedef enum {
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
} log_level_t;

static const char* log_level_names[] = {
    "INFO", "WARN", "ERR "
};

#define MAX_LOG_MESSAGE_LENGTH 2048

void log_message(log_level_t level, const char* format, ...);
void log_memory(log_level_t level, char* data, int data_start, int data_size, const char* func, const char* format, ...);

#ifdef DO_LOG
    #define LOG(level, format, ...) log_message(level, format, ##__VA_ARGS__)
    #define LOGMEM(level, data, start, size, format, ...) log_memory(level, data, start, size, __func__, format, ##__VA_ARGS__)
#else
    #define LOG(level, format, ...)
#endif

void log_debug_message(const char* func, const char* format, ...);
void log_debug_instruction(const char* func, const char* format, ...);

#ifdef DEBUG
    #ifdef DO_DEBUG_LOG
        #define LOGD(format, ...) log_debug_message(__func__, format, ##__VA_ARGS__)
        #define LOGDI(format, ...) log_debug_instruction(__func__, format, ##__VA_ARGS__)
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
