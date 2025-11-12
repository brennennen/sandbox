/*
 * Currently using a "main" and not the callback approach. Comment out "main" and 
 * uncomment "SDL_MAIN_USE_CALLBACKS" to go back to the callback driven approach.
 * 
 * Not sure what the trade offs are, switched as part of trying to get the "close" 
 * 'x' button to work on the frame it's clicked (currently it's processed the 
 * next user interaction (such as a mouse movement) after).
 */
//#define SDL_MAIN_USE_CALLBACKS

#include <stdio.h>

#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_thread.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <libssh/libssh.h>
#include <libssh/sftp.h>

#define CLAY_IMPLEMENTATION
#include "clay.h"

#include "app.h"
#include "app_window.h"
#include "clay_renderer_sdl3.h"
#include "bterm_ui.h"

#include "ssh_manager.h"

static const Uint32 FONT_ID = 0;

static const Clay_Color COLOR_ORANGE    = (Clay_Color) {225, 138, 50, 255};
static const Clay_Color COLOR_BLUE      = (Clay_Color) {111, 173, 162, 255};
static const Clay_Color COLOR_LIGHT     = (Clay_Color) {224, 215, 210, 255};

SDL_Texture *sample_image;

static uint32_t G_SSH_EVENT_TYPE;

#define SSH_EVENT_DIR_LISTING 4

void HandleClayErrors(Clay_ErrorData errorData) {
    printf("%s", errorData.errorText.chars);
}

Uint32 system_timer_long_poll_callback(void *user_data, SDL_TimerID timer_id, uint32_t interval) {
    app_t *app_state = (app_t *) user_data;
    snprintf(app_state->ui_data.fps_text_data, 32, "fps: %3.2f", app_state->fps);
    app_state->ui_data.fps_text = (Clay_String) { 
        .length = strlen(app_state->ui_data.fps_text_data), .chars = app_state->ui_data.fps_text_data 
    };
    return(interval);
}

void log_to_stdout(void* userdata, int category, SDL_LogPriority priority, const char* message) {
    const char* priority_prefix = "[UNKNOWN]";
    switch(priority) {
        case SDL_LOG_PRIORITY_VERBOSE:
            priority_prefix = "[VERBOSE]";
            break;
        case SDL_LOG_PRIORITY_DEBUG:
            priority_prefix = "[DEBUG]";
            break;
        case SDL_LOG_PRIORITY_INFO:
            priority_prefix = "[INFO]";
            break;
        case SDL_LOG_PRIORITY_WARN:
            priority_prefix = "[WARN]";
            break;
        case SDL_LOG_PRIORITY_ERROR:
            priority_prefix = "[ERROR]";
            break;
        case SDL_LOG_PRIORITY_CRITICAL:
            priority_prefix = "[CRITICAL]";
            break;
    }
    printf("%s: %s\n", priority_prefix, message);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    if (!TTF_Init()) {
        return SDL_APP_FAILURE;
    }

    SDL_SetLogOutputFunction(log_to_stdout, NULL);
    SDL_Log("test log.");


    app_t *state = SDL_calloc(1, sizeof(app_t));
    if (!state) {
        return SDL_APP_FAILURE;
    }
    *appstate = state;

    int ssh_event_type = SDL_RegisterEvents(1);
    if (ssh_event_type == (uint32_t)-1) {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Could not register custom event!");
        return SDL_APP_FAILURE;
    }
    state->ssh_event_type = ssh_event_type; 
    ssh_thread_init(state->ssh_event_type);

    SDL_SetAtomicInt(&state->ssh_context.is_running, 1);
    strncpy(state->ssh_context.hostname, "192.168.1.149", sizeof(state->ssh_context.hostname));
    state->ssh_context.connect_requested = 0;

    state->ssh_context.mutex = SDL_CreateMutex();
    state->ssh_context.connect_condition = SDL_CreateCondition();
    if (!state->ssh_context.mutex || !state->ssh_context.connect_condition) {
        SDL_Log("Failed to create ssh mutex/cond: %s", SDL_GetError());
        SDL_Quit();
        return SDL_APP_FAILURE; 
    }

    state->ssh_thread = SDL_CreateThread(
        ssh_thread_function,
        "ssh_thread",
        (void*)&(state->ssh_context)
    );

    state->system_timer_long_poll = SDL_AddTimer(1000, system_timer_long_poll_callback, (void*)state);

    if (!SDL_CreateWindowAndRenderer(
        "Clay Sandbox", 
        1280, 
        720, 
        SDL_WINDOW_BORDERLESS, 
        &state->window.sdl_window, 
        &state->rendererData.renderer)
    ) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetWindowResizable(state->window.sdl_window, true);
    SDL_SetWindowBordered(state->window.sdl_window, false);
    state->window.is_dragging = false;
    state->window.is_resizing = false;
    state->window.resize_edges = 0;
    state->window.cursors = &state->cursors;
    state->cursors.arrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    state->cursors.ns_resize = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NS_RESIZE);
    state->cursors.ew_resize = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_EW_RESIZE);
    state->cursors.nwse_resize = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NWSE_RESIZE);
    state->cursors.nesw_resize = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NESW_RESIZE);

    state->rendererData.textEngine = TTF_CreateRendererTextEngine(state->rendererData.renderer);
    if (!state->rendererData.textEngine) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create text engine from renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    state->rendererData.fonts = SDL_calloc(1, sizeof(TTF_Font *));
    if (!state->rendererData.fonts) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to allocate memory for the font array: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    TTF_Font *font = TTF_OpenFont("resources/Roboto-Regular.ttf", 24);
    if (!font) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load font: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    state->rendererData.fonts[FONT_ID] = font;

    sample_image = IMG_LoadTexture(state->rendererData.renderer, "resources/sample.png");
    if (!sample_image) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load image: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    uint64_t totalMemorySize = Clay_MinMemorySize();
    Clay_Arena clayMemory = (Clay_Arena) {
        .memory = SDL_malloc(totalMemorySize),
        .capacity = totalMemorySize
    };

    int width;
    int height;
    SDL_GetWindowSize(state->window.sdl_window, &width, &height);
    Clay_Initialize(clayMemory, (Clay_Dimensions) { (float) width, (float) height }, (Clay_ErrorHandler) { HandleClayErrors });
    Clay_SetMeasureTextFunction(SDL_MeasureText, state->rendererData.fonts);

    state->ui_data = ui_data_initialize();
    set_ui_global_context(state);

    //Clay_SetDebugModeEnabled(true);
    // turn on vsync to limit fps for now, TODO: something smarter later
    SDL_SetRenderVSync(state->rendererData.renderer, 1);

    *appstate = state;
    return SDL_APP_CONTINUE;
}

static void event_mouse_button_down(app_t *state, SDL_Event *event) {
    if (event->button.button == SDL_BUTTON_LEFT) {
        if (state->is_hovering_side_pane_resizer) {
            printf("clicked while hover pane resizer!\n");
            state->is_resizing_side_pane = true;
            state->action_start_side_pane_width = state->ui_data.sidebar_width;
            float global_x, global_y;
            SDL_GetGlobalMouseState(&global_x, &global_y);
            state->action_start_side_panel_rect.x = (int)global_x;
            state->action_start_side_panel_rect.y = (int)global_y;
        }
        if (state->connect_button_hovering) {
            printf("clicked connect button!\n");
            ssh_request_t* req = create_ssh_request(SSH_COMMAND_CONNECT, NULL);
            ssh_queue_push(&state->ssh_context, req);
        }
    }
}

static void event_mouse_button_up(app_t *state, SDL_Event *event) {
    if (event->button.button == SDL_BUTTON_LEFT) {
        state->is_resizing_side_pane = false;
    }
}

static void event_key_down(app_t *state, SDL_Event *event) {
    if (event->key.key == SDLK_ESCAPE) {
        if (state->is_mouse_locked) {
            state->is_mouse_locked = false;
            SDL_SetWindowRelativeMouseMode(state->window.sdl_window, false);
        }
    }

    if (event->key.key == SDLK_L) {
        SDL_Log("UI: Queuing SSH_CMD_LIST_DIR command for '/home/b'.");
        ssh_request_t* req = create_ssh_request(SSH_COMMAND_LIST_DIR, "/home/b");
        if (req) {
            ssh_queue_push(&state->ssh_context, req);
        }
    }
}

static void manage_cursor(app_t* app, SDL_Event *event) {
    int win_w;
    int win_h;
    app_window_t* window = &app->window;
    SDL_GetWindowSize(app->window.sdl_window, &win_w, &win_h);
    int edges = get_window_resize_edges(event->motion.x, event->motion.y, win_w, win_h);
    SDL_Cursor *cursor_to_set = app->window.cursors->arrow;

    switch (edges) {
        case WINDOW_RESIZE_TOP: cursor_to_set = window->cursors->ns_resize; break;
        case WINDOW_RESIZE_BOTTOM: cursor_to_set = window->cursors->ns_resize; break;
        case WINDOW_RESIZE_LEFT: cursor_to_set = window->cursors->ew_resize; break;
        case WINDOW_RESIZE_RIGHT: cursor_to_set = window->cursors->ew_resize; break;
        case WINDOW_RESIZE_TOP | WINDOW_RESIZE_LEFT: cursor_to_set = window->cursors->nwse_resize; break;
        case WINDOW_RESIZE_BOTTOM | WINDOW_RESIZE_RIGHT: cursor_to_set = window->cursors->nwse_resize; break;
        case WINDOW_RESIZE_TOP | WINDOW_RESIZE_RIGHT: cursor_to_set = window->cursors->nesw_resize; break;
        case WINDOW_RESIZE_BOTTOM | WINDOW_RESIZE_LEFT: cursor_to_set = window->cursors->nesw_resize; break;
        default:
            break;
    }

    if (app->is_hovering_side_pane_resizer) {
        cursor_to_set = window->cursors->ew_resize;
    }

    if (cursor_to_set) {
        SDL_SetCursor(cursor_to_set);
    }
}

static void event_mouse_motion(app_t *state, SDL_Event *event) {
    if (state->is_resizing_side_pane) {
        float global_mouse_x;
        float global_mouse_y;
        SDL_GetGlobalMouseState(&global_mouse_x, &global_mouse_y);
        
        int delta_x = (int)global_mouse_x - state->action_start_side_panel_rect.x;
        state->ui_data.sidebar_width = state->action_start_side_pane_width + delta_x;

        // Add constraints
        int win_w, win_h;
        SDL_GetWindowSize(state->window.sdl_window, &win_w, &win_h);
        if (state->ui_data.sidebar_width < 100) {
            state->ui_data.sidebar_width = 100;
        }
        if (state->ui_data.sidebar_width > win_w - 100) {
            state->ui_data.sidebar_width = win_w - 100;
        }
    }
    manage_cursor(state, event);
}

static void free_file_tree(RemoteFileNode* node) {
    if (node == NULL) return;
    free_file_tree(node->children);
    free_file_tree(node->next_sibling);
    SDL_free(node->name);
    SDL_free(node);
}

static RemoteFileNode* create_root_node(const char* path) {
    RemoteFileNode* node = (RemoteFileNode*)SDL_calloc(1, sizeof(RemoteFileNode));
    if (!node) return NULL;
    
    node->name = SDL_strdup(path); // You could make this "Home" or parse the path
    node->type = REMOTE_FILE_TYPE_DIRECTORY;
    node->is_expanded = true;
    return node;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    app_t *state = appstate;
    SDL_AppResult ret_val = SDL_APP_CONTINUE;

    // Reset certain ui states that are handled in an immediate mode way
    state->is_hovering_side_pane_resizer = false;
    state->connect_button_hovering = false;

    // Let CLAY handle any events it needs to.
    switch (event->type) {
        case SDL_EVENT_WINDOW_RESIZED:
            Clay_SetLayoutDimensions((Clay_Dimensions) { (float) event->window.data1, (float) event->window.data2 });
            break;
        case SDL_EVENT_MOUSE_MOTION: {
            Clay_SetPointerState((Clay_Vector2) { event->motion.x, event->motion.y }, event->motion.state & SDL_BUTTON_LMASK);
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            Clay_SetPointerState((Clay_Vector2) { event->button.x, event->button.y }, event->button.button == SDL_BUTTON_LEFT);
            break;
        }
        case SDL_EVENT_MOUSE_WHEEL:
            Clay_UpdateScrollContainers(true, (Clay_Vector2) { event->wheel.x, event->wheel.y }, 0.01f);
            break;
        default:
            break;
    };

    app_window_sdl_event(&state->window, event);

    if (event->type == state->ssh_event_type) {
        switch (event->user.code) {
            case SSH_EVENT_CONNECTED:
                printf("SSH_EVENT_CONNECTED\n");
                ssh_request_t* req = create_ssh_request(SSH_COMMAND_LIST_DIR, ".");
                if (req) {
                    ssh_queue_push(&state->ssh_context, req);
                }
                break;
            case SSH_EVENT_DIR_LISTING:
                printf("UI: Received SSH_EVENT_DIR_LISTING\n");
                SshDirListingResult* result = (SshDirListingResult*)event->user.data1;
                RemoteFileNode* target_node = state->node_awaiting_children;
                state->node_awaiting_children = NULL; 
                if (result == NULL || !result->success) {
                    printf("bad ssh dir listing result!\n");
                    if (result) SDL_free(result);
                } else {
                    RemoteFileNode* parent_node = NULL; 

                    if (target_node == NULL) {
                        SDL_Log("UI: Attaching children to ROOT");
                        if (state->file_tree_root) {
                            free_file_tree(state->file_tree_root);
                        }
                        state->file_tree_root = create_root_node("."); 
                        state->file_tree_root->children = result->children_head;
                        state->file_tree_root->is_expanded = true;
                        parent_node = state->file_tree_root;

                    } else {
                        SDL_Log("UI: Attaching children to node: %s", target_node->name);
                        target_node->children = result->children_head;
                        target_node->is_expanded = true;
                        parent_node = target_node;
                    }

                    if (parent_node) {
                        RemoteFileNode* child = parent_node->children;
                        while (child) {
                            child->parent = parent_node;
                            child = child->next_sibling;
                        }
                    }
                    SDL_free(result);
                }
                break;
            case SSH_EVENT_DISCONNECTED:
                SDL_Log("UI: Received SSH_EVENT_DISCONNECTED.");
                if (state->file_tree_root) {
                    free_file_tree(state->file_tree_root);
                    state->file_tree_root = NULL;
                }
                state->node_awaiting_children = NULL;
                break;
        }
    }

    switch (event->type) {
        case SDL_EVENT_QUIT:
            ret_val = SDL_APP_SUCCESS;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            event_mouse_button_down(state, event);
            break;
        }
        case SDL_EVENT_MOUSE_MOTION: {
            event_mouse_motion(state, event);
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            event_mouse_button_up(state, event);
            break;
        }
        case SDL_EVENT_KEY_DOWN: {
            event_key_down(state, event);
            break;
        }
        default:
            break;
    };

    return ret_val;
}

void calculate_fps(app_t *state) {
    uint64_t current_time = SDL_GetTicks();
    uint64_t fps_delta_time = current_time - state->last_fps_calc_time;

    if (fps_delta_time >= 1000) {
        state->fps = (float) state->fps_frame_counter / (fps_delta_time / 1000.0f);
        state->fps_frame_counter = 0;
        state->last_fps_calc_time = current_time;
        //printf("fps: %f\n", state->fps);
    }
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    app_t *state = appstate;
    state->fps_frame_counter += 1;
    calculate_fps(state);

    uint64_t now = SDL_GetTicksNS();

    Clay_RenderCommandArray render_commands = ui_create_layout(state);

    SDL_SetRenderDrawColor(state->rendererData.renderer, 0, 0, 0, 255);
    SDL_RenderClear(state->rendererData.renderer);

    SDL_Clay_RenderClayCommands(&state->rendererData, &render_commands);

    SDL_RenderPresent(state->rendererData.renderer);

    if (state->ui_data.should_quit) {
        printf("SDL_AppIterate: should_quit?\n");
        return(SDL_APP_SUCCESS);
    }

    return(SDL_APP_CONTINUE);
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    (void) result;

    if (result != SDL_APP_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Application failed to run");
    }

    app_t *state = appstate;

    if (sample_image) {
        SDL_DestroyTexture(sample_image);
    }

    if (state) {
        if (state->rendererData.renderer)
            SDL_DestroyRenderer(state->rendererData.renderer);

        if (state->window.sdl_window)
            SDL_DestroyWindow(state->window.sdl_window);

        if (state->rendererData.fonts) {
            for(size_t i = 0; i < sizeof(state->rendererData.fonts) / sizeof(*state->rendererData.fonts); i++) {
                TTF_CloseFont(state->rendererData.fonts[i]);
            }
        }

        if (state->rendererData.textEngine)
            TTF_DestroyRendererTextEngine(state->rendererData.textEngine);

        SDL_free(state);
    }
    TTF_Quit();
}

int main(int argc, char* argv[]) {
    app_t *state = NULL;
    SDL_AppResult init_result = SDL_AppInit((void**)&state, argc, argv);
    if (init_result == SDL_APP_FAILURE) {
        printf("SDL_AppInit failed\n");
        return(-1);
    }

    bool quit = false;
    while(!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            SDL_AppEvent((void*)state, &event);
        }
        SDL_AppIterate((void*)state);
        if (state->ui_data.should_quit) {
            quit = true;
        }
        //sleep?
    }
    SDL_AppResult quit_result = SDL_APP_SUCCESS;
    SDL_AppQuit((void*)state, quit_result);
}
