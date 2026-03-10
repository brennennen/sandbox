#include "logger.h"
#include <SDL3/SDL.h>
#include <stdio.h>

static SDL_Mutex* log_mutex = nullptr;

void log_init(void) {
    if (log_mutex == nullptr) {
        log_mutex = SDL_CreateMutex();
    }
}

void log_shutdown(void) {
    if (log_mutex != nullptr) {
        SDL_DestroyMutex(log_mutex);
        log_mutex = nullptr;
    }
}

static void log_internal(const char* level, const char* message) {
    if (log_mutex) SDL_LockMutex(log_mutex);
    printf("[%s]: %s\n", level, message);
    if (log_mutex) SDL_UnlockMutex(log_mutex);
}

void log_info(const char* fmt, ...) {
    if (log_mutex) SDL_LockMutex(log_mutex);
    printf("[INFO]: ");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
    if (log_mutex) SDL_UnlockMutex(log_mutex);
}

void log_warn(const char* fmt, ...) {
    if (log_mutex) SDL_LockMutex(log_mutex);
    printf("[WARN]: ");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
    if (log_mutex) SDL_UnlockMutex(log_mutex);
}

void log_error(const char* fmt, ...) {
    if (log_mutex) SDL_LockMutex(log_mutex);
    fprintf(stderr, "[ERROR]: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    if (log_mutex) SDL_UnlockMutex(log_mutex);
}
