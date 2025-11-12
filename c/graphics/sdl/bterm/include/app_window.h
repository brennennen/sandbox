#ifndef APP_WINDOW_H
#define APP_WINDOW_H

#include <stdbool.h>

#include <SDL3/SDL.h>

#include "app_shared.h"

#define WINDOW_RESIZE_TOP (1 << 0)
#define WINDOW_RESIZE_BOTTOM (1 << 1)
#define WINDOW_RESIZE_LEFT (1 << 2)
#define WINDOW_RESIZE_RIGHT (1 << 3)

typedef struct {
    SDL_Window *sdl_window;
    bool is_dragging;
    bool is_resizing;
    int resize_edges;
    SDL_Rect action_start_window_rect;
    SDL_Point action_start_mouse_pos;

    app_cursors_t* cursors;
} app_window_t;

int get_window_resize_edges(int mouse_x, int mouse_y, int win_w, int win_h);
SDL_AppResult app_window_sdl_event(app_window_t* window, SDL_Event *event);

#endif // APP_WINDOW_H
