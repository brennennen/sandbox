#ifndef APP_H
#define APP_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "clay.h"
#include "clay_renderer_sdl3.h"

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

} ui_data_t;

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
    bool is_resizing_pane;
    float action_start_pane_width;
    SDL_Rect action_start_panel_rect;
    bool is_hovering_pane_resizer;
    float side_panel_width;
    
    app_cursors_t cursors;
} app_t;

#endif // APP_H