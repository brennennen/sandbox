
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>

#include "libraries/emulate8086/include/logger.h"

void log_message(log_level_t level, const char* format, ...) {
    time_t now;
    struct tm timeinfo;
    char timestamp[20];
    char message[MAX_LOG_MESSAGE_LENGTH];

    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", &timeinfo);

    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    fprintf(stdout, "[%s][%s] %s\n", timestamp, log_level_names[level], message);
}

void log_debug_message(const char* func, const char* format, ...) {
    time_t now;
    struct tm timeinfo;
    char timestamp[20];
    char message[MAX_LOG_MESSAGE_LENGTH];

    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", &timeinfo);

    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    fprintf(stdout, "[%s][DEBUG][%s] %s\n", timestamp, func, message);
}

void log_debug_instruction(const char* func, const char* format, ...) {
    char message[MAX_LOG_MESSAGE_LENGTH];
    
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    fprintf(stdout, "[i] %s\n", message);
}


void log_memory(log_level_t level, char* data, int data_start, int data_size, const char* func, const char* format, ...) {
    time_t now;
    struct tm timeinfo;
    char timestamp[20];
    char message[MAX_LOG_MESSAGE_LENGTH];

    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", &timeinfo);

    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    fprintf(stdout, "[%s][%s][%s] %s\n", timestamp, log_level_names[level], func, message);

    int count = 0;
    int data_address = data_start;
    int data_offset = data_start;
    for(int i = 0; i < 4; i++) {
        fprintf(stdout, "%06X   ", data_address);
        for(int i = 0; i < 16; i++) {
            if (data_offset >= data_size) {
                break;
            }
            fprintf(stdout, "%02X ", (uint8_t)*(data + data_offset));
            data_offset += 1;
        }
        data_address += 16;
        //fprintf(stdout, "   ................\n"); // TODO: cast each to ascii
        fprintf(stdout, "\n");
    }
    fprintf(stdout, "\n");
}
