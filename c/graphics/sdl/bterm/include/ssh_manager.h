#ifndef SSH_MANAGER_H
#define SSH_MANAGER_H

#include <libssh/libssh.h>
#include <libssh/sftp.h>

#include <SDL3/SDL.h>
#include <stdbool.h>
#include "clay.h"


#define SSH_EVENT_CONNECTED 1
#define SSH_EVENT_DATA_RECV 2
#define SSH_EVENT_DISCONNECTED 3
#define SSH_EVENT_DIR_LISTING 4

typedef enum {
    SSH_COMMAND_CONNECT,
    SSH_COMMAND_LIST_DIR,
    SSH_COMMAND_DISCONNECT,
    SSH_COMMAND_QUIT
} ssh_command_type_t;

typedef struct ssh_request_s {
    ssh_command_type_t type;
    char* path;
    struct ssh_request_s* next;
} ssh_request_t;

typedef struct {
    ssh_session session;
    SDL_AtomicInt is_running;

    SDL_Mutex* queue_mutex;
    SDL_Condition* queue_cond;
    
    ssh_request_t* queue_head;
    ssh_request_t* queue_tail;

    SDL_Mutex* mutex;
    SDL_Condition* connect_condition;
    int connect_requested;

    char user[128];
    char hostname[128];
    int port;

    ssh_channel channel;
    bool is_connected;
} ssh_context_t;

void ssh_thread_init(uint32_t sdl_event_type);

int ssh_thread_function(void* data);

ssh_request_t* create_ssh_request(ssh_command_type_t type, const char* path);

void ssh_queue_push(ssh_context_t* context, ssh_request_t* request);

#endif // SSH_MANAGER_H
