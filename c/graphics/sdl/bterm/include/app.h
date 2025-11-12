#ifndef APP_H
#define APP_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <libssh/libssh.h>

#include "clay.h"
#include "clay_renderer_sdl3.h"

#include "ssh_manager.h"

#include "app_window.h"

typedef struct {
    char* offset;
    char* memory;
} clay_frame_arena_t;

typedef struct {
    clay_frame_arena_t frameArena;
    int32_t selectedDocumentIndex;
    float yOffset;
    bool should_quit;
    char fps_text_data[32];
    Clay_String fps_text;
    int sidebar_width;

    char connect_string_data[256];
    Clay_String connect_string;

} ui_data_t;

typedef struct {
    Clay_ElementId active_id;
    Clay_String* active_string;
    size_t active_buffer_size;
    int cursor_pos;
    uint64_t cursor_last_blink_time;
    bool cursor_visible;
} text_input_state_t;

typedef enum {
    REMOTE_FILE_TYPE_REGULAR,
    REMOTE_FILE_TYPE_DIRECTORY,
    REMOTE_FILE_TYPE_FILE,
    REMOTE_FILE_TYPE_SYMLINK,
    REMOTE_FILE_TYPE_OTHER
} RemoteFileType;

/*
 * A node in the remote file tree.
 * Uses "first child, next sibling" to represent the tree.
 */
typedef struct RemoteFileNode {
    char* name;
    RemoteFileType type;
    uint64_t size;
    uint32_t mod_time;
    // You can add permissions, owner, etc. here

    // --- Tree topology ---
    struct RemoteFileNode* parent;
    struct RemoteFileNode* children;     // POINTER TO THE FIRST CHILD
    struct RemoteFileNode* next_sibling; // POINTER TO THE NEXT SIBLING
    
    // --- UI State ---
    bool is_expanded; // So the UI knows if it should show children

} RemoteFileNode;

typedef struct {
    bool success;
    char* error_message; // If success == false
    
    // The head of a *new* linked list of children
    // (linked via the `next_sibling` pointer)
    RemoteFileNode* children_head; 

} SshDirListingResult;

typedef struct {
    // todo:
    // ssh_session, vterm_instance, file_tree, text_editor

    Clay_SDL3RendererData rendererData;

    //bterm_ui_data_t bterm_ui_data;
    ui_data_t ui_data;

    uint64_t last_frame_time;
    uint64_t last_fps_calc_time;
    int fps_frame_counter;

    float fps;

    // timers
    SDL_TimerID system_timer_long_poll;

    bool is_mouse_locked;

    app_window_t window;

    // pane resizing
    bool is_resizing_side_pane;
    float action_start_side_pane_width;
    SDL_Rect action_start_side_panel_rect;
    bool is_hovering_side_pane_resizer;
    float side_panel_width;
    
    app_cursors_t cursors;

    uint32_t ssh_event_type;
    ssh_context_t ssh_context;
    SDL_Thread* ssh_thread;
    
    text_input_state_t text_input_state;

    bool connect_button_hovering;

    RemoteFileNode* node_awaiting_children;

    RemoteFileNode* file_tree_root;

} app_t;

#endif // APP_H