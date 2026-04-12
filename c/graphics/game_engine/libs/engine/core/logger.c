
// #include <SDL3/SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "engine/core/logger.h"
#include "engine/platform/platform.h"
#include "engine/platform/platform_mutex.h"

platform_mutex_t log_mutex = NULL;

void log_init(void) {
    if (log_mutex == nullptr) {
        log_mutex = platform_mutex_create();
    }
}

void log_shutdown(void) {
    if (log_mutex != nullptr) {
        platform_mutex_destroy(log_mutex);
        log_mutex = nullptr;
    }
}

static void print_timestamp(FILE* stream) {
    time_t     rawtime;
    struct tm* timeinfo;
    char       buffer[32];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);

    fprintf(stream, "[%s] ", buffer);
}

static void log_internal(const char* level, const char* message) {
    if (log_mutex)
        platform_mutex_lock(log_mutex);

    print_timestamp(stdout);
    printf("[%s]: %s\n", level, message);

    if (log_mutex)
        platform_mutex_unlock(log_mutex);
}

void log_info(const char* fmt, ...) {
    if (log_mutex)
        platform_mutex_lock(log_mutex);

    print_timestamp(stdout);
    printf("[INFO]: ");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");

    if (log_mutex)
        platform_mutex_unlock(log_mutex);
}

void log_warn(const char* fmt, ...) {
    if (log_mutex)
        platform_mutex_lock(log_mutex);

    print_timestamp(stdout);
    printf("[WARN]: ");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");

    if (log_mutex)
        platform_mutex_unlock(log_mutex);
}

void log_error(const char* fmt, ...) {
    if (log_mutex)
        platform_mutex_lock(log_mutex);

    print_timestamp(stderr);
    fprintf(stderr, "[ERROR]: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");

    if (log_mutex)
        platform_mutex_unlock(log_mutex);
}
