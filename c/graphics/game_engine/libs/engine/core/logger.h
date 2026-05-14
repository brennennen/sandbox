#pragma once

#include <stdbool.h>

typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_NONE,
    LOG_LEVEL_COUNT
} log_level_t;

typedef enum {
    LOG_COMMON = 0,
    LOG_CORE,
    LOG_GFX,
    LOG_PHYSICS,
    LOG_AUDIO,
    LOG_GAME,
    LOG_CHANNEL_COUNT
} log_channel_t;

void log_init(void);
void log_shutdown(void);

void log_debug_channel(log_channel_t channel, const char* fmt, ...);
void log_info_channel(log_channel_t channel, const char* fmt, ...);
void log_warn_channel(log_channel_t channel, const char* fmt, ...);
void log_error_channel(log_channel_t channel, const char* fmt, ...);

#ifndef LOG_CURRENT_CHANNEL
#define LOG_CURRENT_CHANNEL LOG_COMMON
#endif

#define log_debug(fmt, ...) log_debug_channel(LOG_CURRENT_CHANNEL, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...) log_info_channel(LOG_CURRENT_CHANNEL, fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...) log_warn_channel(LOG_CURRENT_CHANNEL, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...) log_error_channel(LOG_CURRENT_CHANNEL, fmt, ##__VA_ARGS__)

int         logger_get_history_count(void);
const char* logger_get_history_line(int index);
