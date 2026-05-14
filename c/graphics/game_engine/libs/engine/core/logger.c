
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include <SDL3/SDL_time.h>

#include "engine/core/logger.h"
#include "engine/platform/platform.h"

static constexpr uint32_t LOG_MAX_LINES  = 1024;
static constexpr uint32_t LOG_MAX_LENGTH = 256;

platform_mutex_t log_mutex = NULL;

static log_level_t channel_levels[LOG_CHANNEL_COUNT] = {
    [LOG_COMMON]  = LOG_LEVEL_INFO,
    [LOG_CORE]    = LOG_LEVEL_INFO,
    [LOG_GFX]     = LOG_LEVEL_INFO,
    [LOG_PHYSICS] = LOG_LEVEL_INFO,
    [LOG_AUDIO]   = LOG_LEVEL_INFO,
    [LOG_GAME]    = LOG_LEVEL_INFO,
};

static const char* channel_names[LOG_CHANNEL_COUNT] = {
    "_",
    "CORE",
    "GFX",
    "PHYS",
    "AUDIO",
    "GAME",
};

static const char* level_names[LOG_LEVEL_COUNT] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "NONE",
};

typedef struct {
    char log_ring_buffer[LOG_MAX_LINES][LOG_MAX_LENGTH];
    int  log_head;
    int  log_count;
} logger_t;
static logger_t logger;

void log_init(void) {
    if (log_mutex == nullptr) {
        log_mutex = platform_mutex_create();
    }
}

void log_set_channel_level(log_channel_t channel, log_level_t level) {
    if (channel >= 0 && channel < LOG_CHANNEL_COUNT) {
        channel_levels[channel] = level;
    }
}

void log_set_all_levels(log_level_t level) {
    for (int i = 0; i < LOG_CHANNEL_COUNT; i++) {
        channel_levels[i] = level;
    }
}

void log_shutdown(void) {
    if (log_mutex != nullptr) {
        platform_mutex_destroy(log_mutex);
        log_mutex = nullptr;
    }
}

static void log_internal(
    log_channel_t channel,
    log_level_t   level,
    FILE*         stream,
    const char*   fmt,
    va_list       args
) {
    if (level < channel_levels[channel]) {
        return;
    }

    char message_buffer[LOG_MAX_LENGTH];
    vsnprintf(message_buffer, sizeof(message_buffer), fmt, args);

    if (log_mutex) {
        platform_mutex_lock(log_mutex);
    }

    SDL_Time ticks;
    SDL_GetCurrentTime(&ticks);
    SDL_DateTime dt;
    SDL_TimeToDateTime(ticks, &dt, true);
    char time_buffer[32];
    snprintf(time_buffer, sizeof(time_buffer), "%02d:%02d:%02d", dt.hour, dt.minute, dt.second);

    const char* ch_name    = channel_names[channel];
    const char* level_name = level_names[level];

    char* target_buffer = logger.log_ring_buffer[logger.log_head];
    if (channel == LOG_COMMON) {
        snprintf(
            target_buffer, LOG_MAX_LENGTH, "[%s] [%s]: %s", time_buffer, level_name, message_buffer
        );
    } else {
        snprintf(
            target_buffer,
            LOG_MAX_LENGTH,
            "[%s] [%s] [%s]: %s",
            time_buffer,
            ch_name,
            level_name,
            message_buffer
        );
    }

    fputs(target_buffer, stream);
    fputc('\n', stream);
    fflush(stream);

    logger.log_head = (logger.log_head + 1) % LOG_MAX_LINES;
    if (logger.log_count < LOG_MAX_LINES) {
        logger.log_count++;
    }

    if (log_mutex) {
        platform_mutex_unlock(log_mutex);
    }
}

void log_debug_channel(log_channel_t channel, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_internal(channel, LOG_LEVEL_DEBUG, stdout, fmt, args);
    va_end(args);
}

void log_info_channel(log_channel_t channel, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_internal(channel, LOG_LEVEL_INFO, stdout, fmt, args);
    va_end(args);
}

void log_warn_channel(log_channel_t channel, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_internal(channel, LOG_LEVEL_WARN, stdout, fmt, args);
    va_end(args);
}

void log_error_channel(log_channel_t channel, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_internal(channel, LOG_LEVEL_ERROR, stdout, fmt, args);
    va_end(args);
}
