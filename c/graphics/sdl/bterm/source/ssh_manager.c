
#include <stdint.h>
#include <stdio.h>

#include <libssh/libssh.h>
#include <libssh/sftp.h>

#include "app.h"


static uint32_t G_SSH_EVENT_TYPE;


void ssh_thread_init(uint32_t sdl_event_type) {
    G_SSH_EVENT_TYPE = sdl_event_type;
}

ssh_request_t* create_ssh_request(ssh_command_type_t type, const char* path) {
    ssh_request_t* req = (ssh_request_t*)SDL_malloc(sizeof(ssh_request_t));
    if (!req) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to allocate ssh_request_t");
        return NULL;
    }
    req->type = type;
    req->path = path ? SDL_strdup(path) : NULL;
    req->next = NULL;
    return req;
}

void ssh_queue_push(ssh_context_t* context, ssh_request_t* request) {
    if (request == NULL) {
        return;
    }
    SDL_LockMutex(context->queue_mutex);
    
    if (context->queue_tail) {
        context->queue_tail->next = request;
        context->queue_tail = request;
    } else {
        context->queue_head = request;
        context->queue_tail = request;
    }
    SDL_SignalCondition(context->queue_cond);
    SDL_UnlockMutex(context->queue_mutex);
}

ssh_request_t* ssh_queue_pop(ssh_context_t* context) {
    SDL_LockMutex(context->queue_mutex);
    
    while (context->queue_head == NULL && SDL_GetAtomicInt(&context->is_running)) {
        SDL_WaitCondition(context->queue_cond, context->queue_mutex);
    }
    
    ssh_request_t* req = NULL;
    if (context->queue_head) {
        req = context->queue_head;
        context->queue_head = req->next;
        
        if (context->queue_head == NULL) {
            context->queue_tail = NULL;
        }
    }
    
    SDL_UnlockMutex(context->queue_mutex);
    return req;
}

void ssh_queue_cleanup(ssh_context_t* context) {
    ssh_request_t* req;
    while ((req = ssh_queue_pop(context)) != NULL) {
        SDL_Log("Cleaning up queued request: %d", req->type);
        SDL_free(req->path);
        SDL_free(req);
    }
}

static void ssh_handle_connect(ssh_context_t* context) {
    if (context->is_connected) {
        SDL_Log("SSH: Already connected.");
        return;
    }
    
    SDL_Log("SSH: Connecting...");
    context->session = ssh_new();
    if (!context->session) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SSH: ssh_new() failed.");
        return;
    }

    ssh_options_set(context->session, SSH_OPTIONS_HOST, "192.168.1.149");
    ssh_options_set(context->session, SSH_OPTIONS_USER, "b");

    int connect_rc = ssh_connect(context->session);
    if (connect_rc != SSH_OK) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SSH connection error: %s", 
             ssh_get_error(context->session));
        
        SDL_Event event;
        SDL_zero(event);
        event.type = G_SSH_EVENT_TYPE;
        event.user.code = SSH_EVENT_DISCONNECTED;
        SDL_PushEvent(&event);

        ssh_free(context->session);
        context->session = NULL;
        return;
    }
    
    SDL_Log("SSH: Connected!");
    
    int auth_rc = ssh_userauth_publickey_auto(context->session, "b", NULL);
    if (auth_rc != SSH_AUTH_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error authenticating: %s", 
            ssh_get_error(context->session));
        ssh_disconnect(context->session);
        ssh_free(context->session);
        context->session = NULL;
        return;
    }
    
    SDL_Log("SSH: Authenticated!");
    context->is_connected = true;

    SDL_Event connect_event;
    SDL_zero(connect_event);
    connect_event.type = G_SSH_EVENT_TYPE;
    connect_event.user.code = SSH_EVENT_CONNECTED;
    SDL_PushEvent(&connect_event);
}

static void ssh_handle_list_dir(ssh_context_t* context, const char* path) {
    printf("%s\n", __func__);
    if (!context->is_connected || !context->session) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "SSH: Not connected, cannot list dir.");
        return;
    }

    SshDirListingResult* result = (SshDirListingResult*)SDL_calloc(1, sizeof(SshDirListingResult));
    if (!result) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to alloc SshDirListingResult");
        return;
    }
    result->success = false;

    sftp_session sftp = sftp_new(context->session);
    if (sftp == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SFTP: Error allocating session: %s",
             ssh_get_error(context->session));
        return;
    }

    if (sftp_init(sftp) != SSH_OK) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SFTP: Error initializing session: %s",
             ssh_get_error(context->session));
        sftp_free(sftp);
        return;
    }

    const char* dir_path = (path && path[0] != '\0') ? path : ".";
    SDL_Log("SSH: Listing directory via SFTP: %s", dir_path);

    sftp_dir dir = sftp_opendir(sftp, dir_path);
    if (dir == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SFTP: Can't open dir: %s",
             ssh_get_error(context->session));
        sftp_free(sftp);
        return;
    }

    RemoteFileNode* list_head = NULL;
    RemoteFileNode* list_tail = NULL;
    sftp_attributes attributes;

    while ((attributes = sftp_readdir(sftp, dir)) != NULL) {
        if (strcmp(attributes->name, ".") == 0 || strcmp(attributes->name, "..") == 0) {
            sftp_attributes_free(attributes);
            continue;
        }

        SDL_Log("  > %s (type: %d, size: %llu)", 
            attributes->name, 
            attributes->type, 
            (unsigned long long)attributes->size
        );
        
        RemoteFileNode* new_node = (RemoteFileNode*)SDL_calloc(1, sizeof(RemoteFileNode));
        if (!new_node) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to allocate RemoteFileNode");
            sftp_attributes_free(attributes);
            continue; 
        }

        new_node->name = SDL_strdup(attributes->name);
        if (attributes->type == SSH_FILEXFER_TYPE_DIRECTORY) {
            new_node->type = REMOTE_FILE_TYPE_DIRECTORY;
        } else {
            new_node->type = REMOTE_FILE_TYPE_FILE;
        }

        if (list_tail == NULL) {
            list_head = new_node;
            list_tail = new_node;
        } else {
            list_tail->next_sibling = new_node;
            list_tail = new_node;
        }

        sftp_attributes_free(attributes);
    }

    if (!sftp_dir_eof(dir)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SFTP: Can't read dir: %s",
             ssh_get_error(context->session));
    }

    sftp_closedir(dir);
    sftp_free(sftp);
    SDL_Log("SSH: Finished listing directory.");

    result->success = true;

push_event:
    result->children_head = list_head; 

    SDL_Event event;
    SDL_zero(event);
    event.type = G_SSH_EVENT_TYPE;
    event.user.code = SSH_EVENT_DIR_LISTING;
    event.user.data1 = (void*)result;
    SDL_PushEvent(&event);
}

static void ssh_handle_disconnect(ssh_context_t* context) {
    if (!context->is_connected) {
        SDL_Log("SSH: Already disconnected.");
        return;
    }
    
    SDL_Log("SSH: Disconnecting...");
    if (context->channel) {
        ssh_channel_close(context->channel);
        ssh_channel_free(context->channel);
        context->channel = NULL;
    }
    if (context->session) {
        ssh_disconnect(context->session);
        ssh_free(context->session);
        context->session = NULL;
    }
    context->is_connected = false;
    SDL_Log("SSH: Disconnected.");

    SDL_Event event;
    SDL_zero(event);
    event.type = G_SSH_EVENT_TYPE;
    event.user.code = SSH_EVENT_DISCONNECTED;
    SDL_PushEvent(&event);
}

int ssh_thread_function(void* data) {
    SDL_Log("ssh_thread_function started.");
    ssh_context_t* context = (ssh_context_t*)data;

    while (SDL_GetAtomicInt(&context->is_running)) {
        ssh_request_t* req = ssh_queue_pop(context);

        if (!req) {
            continue;
        }

        SDL_Log("SSH Thread processing command: %d", req->type);

        switch (req->type) {
            case SSH_COMMAND_CONNECT:
                ssh_handle_connect(context);
                break;
            
            case SSH_COMMAND_LIST_DIR:
                ssh_handle_list_dir(context, req->path);
                break;

            case SSH_COMMAND_DISCONNECT:
                ssh_handle_disconnect(context);
                break;

            case SSH_COMMAND_QUIT:
                SDL_Log("SSH Thread received QUIT command.");
                SDL_SetAtomicInt(&context->is_running, 0);
                break;
            
            default:
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unknown SSH command: %d", req->type);
                break;
        }

        SDL_free(req->path);
        SDL_free(req);
    }

    if (context->is_connected) {
        ssh_handle_disconnect(context);
    }
    ssh_queue_cleanup(context);
    
    SDL_Log("SSH thread shutting down...");
    return 0;
}

int ssh_thread_function2(void* data) {
    printf("ssh_thread_function?\n");
    ssh_context_t* context = (ssh_context_t*)data;
    context->session = ssh_new();

    SDL_LockMutex(context->mutex);
    while (!context->connect_requested && SDL_GetAtomicInt(&context->is_running)) {
        SDL_WaitCondition(context->connect_condition, context->mutex);
    }
    SDL_UnlockMutex(context->mutex);

    if (!SDL_GetAtomicInt(&context->is_running)) {
        SDL_Log("SSH thread woken up to quit.");
        ssh_free(context->session);
        return 0;
    }

    // todo: set options
    ssh_options_set(context->session, SSH_OPTIONS_HOST, "192.168.1.149");
    ssh_options_set(context->session, SSH_OPTIONS_USER, "b");
    //ssh_options_set(context->session, SSH_OPTIONS_LOG_VERBOSITY, );

    int connect_rc = ssh_connect(context->session);
    if (connect_rc != SSH_OK) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SSH connection error: %s", 
            ssh_get_error(context->session));
        
        SDL_Event event;
        SDL_zero(event);
        event.type = G_SSH_EVENT_TYPE;
        event.user.code = SSH_EVENT_DISCONNECTED;
        SDL_PushEvent(&event);

        ssh_free(context->session);
        return 1;
    }
    printf("connected!\n");
    int auth_rc = ssh_userauth_publickey_auto(context->session, "b", NULL);
    if (auth_rc == SSH_AUTH_SUCCESS) {
        printf("authenticated!\n");
    } else {
        fprintf(stderr, "Error authenticating with public key: %s\n", 
            ssh_get_error(context->session)
        );
        ssh_free(context->session);
        return 1;
    }

    context->channel = ssh_channel_new(context->session);
    if (ssh_channel_open_session(context->channel) != SSH_OK) {
        fprintf(stderr, "Failed to open channel session: %s\n",
            ssh_get_error(context->session)
        );
        ssh_channel_free(context->channel);
        ssh_disconnect(context->session);
        ssh_free(context->session);
        return(-1);
    }
    printf("channel opened!\n");

    if (ssh_channel_request_exec(context->channel, "ls") != SSH_OK) {
        fprintf(stderr, "Failed to execute command: %s\n",
            ssh_get_error(context->session)
        );
        ssh_channel_close(context->channel);
        ssh_channel_free(context->channel);
        ssh_disconnect(context->session);
        ssh_free(context->session);
        return(-1);
    }
    printf("Output:\n");
    char buffer[256];
    int buffer_size = sizeof(buffer);
    int nbytes = ssh_channel_read(context->channel, buffer, buffer_size, 0);
    while (nbytes > 0) {
        if (fwrite(buffer, 1, nbytes, stdout) != (size_t)nbytes) {
            fprintf(stderr, "Error writing to stdout\n");
            ssh_channel_close(context->channel);
            return(-1);
        }
        nbytes = ssh_channel_read(context->channel, buffer, buffer_size, 0);
    }
    printf("\n");
    if (nbytes < 0) {
        fprintf(stderr, "Error reading from channel: %s\n",
            ssh_get_error(context->session)
        );
        return(-1);
    }

    SDL_Event connect_event;
    SDL_zero(connect_event);
    connect_event.type = G_SSH_EVENT_TYPE;
    connect_event.user.code = SSH_EVENT_CONNECTED;
    SDL_PushEvent(&connect_event);

    while(SDL_GetAtomicInt(&context->is_running)) {
        SDL_Delay(50);
    }

    SDL_Log("SSH thread shutting down...");
    ssh_free(context->session);
    return 0;
}
