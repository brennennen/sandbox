#ifndef PLATFORM_MUTEX_H
#define PLATFORM_MUTEX_H

typedef void* platform_mutex_t;

platform_mutex_t platform_mutex_create(void);
void             platform_mutex_lock(platform_mutex_t mutex);
void             platform_mutex_unlock(platform_mutex_t mutex);
void             platform_mutex_destroy(platform_mutex_t mutex);

#endif
