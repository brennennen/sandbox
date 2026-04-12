#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>

void log_init(void);
void log_shutdown(void);

void log_info(const char* fmt, ...);
void log_warn(const char* fmt, ...);
void log_error(const char* fmt, ...);

#endif
