/**
Platform layer


*/

#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>

#include "libs/core/string_span.h"

typedef struct platform_t platform_t;

typedef enum {
    KEY_Q,
    KEY_W,
    KEY_E,
    KEY_R,
    KEY_T,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_F,
    KEY_G,
    KEY_SPACE,
    KEY_LSHIFT,
    KEY_LCTRL,
    KEY_LALT,
    KEY_P,
    KEY_ESCAPE,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_MAX
} platform_key_t;

platform_t* platform_create(const char* title, int width, int height);
void        platform_destroy(platform_t* p);

bool     platform_update(platform_t* p);
void     platform_get_window_size(platform_t* p, int* w, int* h);
void     platform_set_title(platform_t* p, const char* title);
void     platform_set_relative_mouse(platform_t* p, bool enable);
void*    platform_get_native_window(platform_t* p);
bool     platform_get_key(platform_t* p, platform_key_t key);
void     platform_get_mouse_delta(platform_t* p, float* dx, float* dy);
uint64_t platform_get_ticks(platform_t* p);
bool     platform_get_key(platform_t* platform, platform_key_t key_code);
bool     platform_get_key_pressed(platform_t* platform, platform_key_t key_code);
bool     platform_get_key_released(platform_t* platform, platform_key_t key_code);

// MARK: mutex/semaphores
typedef void*    platform_mutex_t;
platform_mutex_t platform_mutex_create(void);
void             platform_mutex_lock(platform_mutex_t mutex);
void             platform_mutex_unlock(platform_mutex_t mutex);
void             platform_mutex_destroy(platform_mutex_t mutex);

// MARK: atomics
typedef struct {
    int value;
} platform_atomic_int_t;
void platform_atomic_int_set(platform_atomic_int_t* atomic, int value);
int  platform_atomic_int_add(platform_atomic_int_t* atomic, int value);
int  platform_atomic_int_get(platform_atomic_int_t* atomic);

// MARK: threading
typedef void* platform_thread_t;
typedef int (*platform_thread_func)(void* data);

int               platform_get_core_count();
platform_thread_t platform_thread_create(platform_thread_func fn, const char* name, void* data);
void              platform_thread_wait(platform_thread_t thread);

// MARK: Files/Directories/Paths
void platform_create_directory(string_span_t path);

// MARK: Files & Memory Mapping
// An opaque OS file handle for 64-bit safe streaming/reading
typedef void* platform_file_t;

// A memory mapped file
typedef struct {
    void*           data;       // The actual pointer to the file in RAM
    uint64_t        size;       // Total size of the file
    void*           _os_handle; // Internal OS mapping handle (hMap on Win32, fd on POSIX)
    platform_file_t _file;      // Internal OS file handle
} platform_file_mapping_t;

// Memory Mapping (For the VFS)
bool platform_file_map_read(const char* filepath, platform_file_mapping_t* out_mapping);
void platform_file_unmap(platform_file_mapping_t* mapping);

// 64-Bit Safe File IO (For the Cooker / Background Streaming)
platform_file_t platform_file_open_read(const char* filepath);
platform_file_t platform_file_open_write(const char* filepath);
void            platform_file_close(platform_file_t file);

uint64_t platform_file_size(platform_file_t file);
uint64_t platform_file_read(platform_file_t file, void* buffer, uint64_t bytes_to_read);
uint64_t platform_file_write(platform_file_t file, const void* buffer, uint64_t bytes_to_write);

bool     platform_file_seek(platform_file_t file, uint64_t offset);
uint64_t platform_file_tell(platform_file_t file);

// MARK: misc

// TODO: wrap this in #ifdef RENDERER_VULKAN?
// #ifdef RENDERER_VULKAN
typedef struct VkInstance_T*   VkInstance;
typedef struct VkSurfaceKHR_T* VkSurfaceKHR;
bool platform_create_vulkan_surface(platform_t* p, VkInstance instance, VkSurfaceKHR* surface);
const char* const* platform_get_vulkan_extensions(platform_t* p, uint32_t* count);
// #endif

#endif // PLATFORM_H
