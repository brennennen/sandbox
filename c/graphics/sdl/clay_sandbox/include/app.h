#ifndef APP_H
#define APP_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "clay.h"

#include "clay_renderer_sdl3.h"
#include "game.h"

typedef struct {
    char* offset;
    char* memory;
} clay_sandbox_arena_t;

typedef struct {
    clay_sandbox_arena_t frameArena;
    int32_t selectedDocumentIndex;
    float yOffset;
    bool should_quit;
    char fps_text_data[32];
    Clay_String fps_text;
    SDL_Texture *game_texture;
} clay_sandbox_ui_data_t;

typedef struct app_state {
    SDL_Window *window;
    Clay_SDL3RendererData rendererData;
    clay_sandbox_ui_data_t sandbox_ui_data;

    uint64_t last_frame_time;
    uint64_t last_fps_calc_time;
    int fps_frame_counter;

    float fps;

    game_state_t game_state;
    uint64_t last_game_update_time;

    // timers
    SDL_TimerID system_timer_long_poll;

    bool is_mouse_locked;

    // moving and resizing
    bool is_dragging;
    bool is_resizing;
    int resize_edges;
    SDL_Rect action_start_window_rect;
    SDL_Point action_start_mouse_pos;

    // cursors
    SDL_Cursor *cursor_arrow;
    SDL_Cursor *cursor_ns_resize;
    SDL_Cursor *cursor_ew_resize;
    SDL_Cursor *cursor_nwse_resize;
    SDL_Cursor *cursor_nesw_resize;
} app_state_t;

#endif // APP_H