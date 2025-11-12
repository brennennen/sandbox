

#include <SDL3/SDL.h>

#include "app_window.h"

#define RESIZE_BORDER_THICKNESS 8
#define DRAG_HANDLE_HEIGHT 30

int get_window_resize_edges(int mouse_x, int mouse_y, int win_w, int win_h) {
    int edges = 0;
    if (mouse_x < RESIZE_BORDER_THICKNESS) {
        edges |= WINDOW_RESIZE_LEFT;
    }
    if (mouse_x >= win_w - RESIZE_BORDER_THICKNESS) {
        edges |= WINDOW_RESIZE_RIGHT;
    }
    if (mouse_y < RESIZE_BORDER_THICKNESS) {
        edges |= WINDOW_RESIZE_TOP;
    }
    if (mouse_y >= win_h - RESIZE_BORDER_THICKNESS) {
        edges |= WINDOW_RESIZE_BOTTOM;
    }
    return edges;
}

static void manage_window_resize_cursor(app_window_t* window, SDL_Event *event) {
    int win_w;
    int win_h;
    SDL_GetWindowSize(window->sdl_window, &win_w, &win_h);
    int edges = get_window_resize_edges(event->motion.x, event->motion.y, win_w, win_h);
    SDL_Cursor *cursor_to_set = window->cursors->arrow;
    //SDL_Cursor *cursor_to_set = NULL;

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
    if (cursor_to_set) {
        SDL_SetCursor(cursor_to_set);
    }
}

static void event_mouse_button_up(app_window_t *window, SDL_Event *event) {
    if (event->button.button == SDL_BUTTON_LEFT) {
        window->is_dragging = false;
        window->is_resizing = false;
        window->resize_edges = 0;
    }
}

static void event_mouse_button_down(app_window_t *window, SDL_Event *event) {
    if (event->button.button == SDL_BUTTON_LEFT) {
        int win_w;
        int win_h;
        SDL_GetWindowSize(window->sdl_window, &win_w, &win_h);
        int edges = get_window_resize_edges(event->button.x, event->button.y, win_w, win_h);

        float global_mouse_x;
        float global_mouse_y;
        SDL_GetGlobalMouseState(&global_mouse_x, &global_mouse_y);
        window->action_start_mouse_pos.x = (int) global_mouse_x;
        window->action_start_mouse_pos.y = (int) global_mouse_y;

        SDL_GetWindowPosition(window->sdl_window, &window->action_start_window_rect.x, &window->action_start_window_rect.y);
        SDL_GetWindowSize(window->sdl_window, &window->action_start_window_rect.w, &window->action_start_window_rect.h);

        if (edges != 0) {
            window->is_resizing = true;
            window->resize_edges = edges;
        } else if (event->button.y < DRAG_HANDLE_HEIGHT) {
            window->is_dragging = true;
        }
    }
}

static void event_mouse_motion(app_window_t* window, SDL_Event *event, bool set_cursor) {
    if (window->is_dragging) {
        float global_mouse_x;
        float global_mouse_y;
        SDL_GetGlobalMouseState(&global_mouse_x, &global_mouse_y);
        int delta_x = (int)global_mouse_x - window->action_start_mouse_pos.x;
        int delta_y = (int)global_mouse_y - window->action_start_mouse_pos.y;

        int new_x = window->action_start_window_rect.x + delta_x;
        int new_y = window->action_start_window_rect.y + delta_y;

        SDL_SetWindowPosition(window->sdl_window, new_x, new_y);
    } else if (window->is_resizing) {
        float global_mouse_x;
        float global_mouse_y;
        SDL_GetGlobalMouseState(&global_mouse_x, &global_mouse_y);
        
        int delta_x = (int)global_mouse_x - window->action_start_mouse_pos.x;
        int delta_y = (int)global_mouse_y - window->action_start_mouse_pos.y;

        int new_x = window->action_start_window_rect.x;
        int new_y = window->action_start_window_rect.y;
        int new_w = window->action_start_window_rect.w;
        int new_h = window->action_start_window_rect.h;

        if (window->resize_edges & WINDOW_RESIZE_LEFT) {
            new_x += delta_x;
            new_w -= delta_x;
        }
        if (window->resize_edges & WINDOW_RESIZE_RIGHT) {
            new_w += delta_x;
        }
        if (window->resize_edges & WINDOW_RESIZE_TOP) {
            new_y += delta_y;
            new_h -= delta_y;
        }
        if (window->resize_edges & WINDOW_RESIZE_BOTTOM) {
            new_h += delta_y;
        }

        SDL_SetWindowPosition(window->sdl_window, new_x, new_y);
        SDL_SetWindowSize(window->sdl_window, new_w, new_h);
    } 
    // else {
    //     manage_window_resize_cursor(window, event);
    // }
}

SDL_AppResult app_window_sdl_event(app_window_t* window, SDL_Event *event) {
    SDL_AppResult ret_val = SDL_APP_CONTINUE;

    switch (event->type) {
        case SDL_EVENT_QUIT:
            ret_val = SDL_APP_SUCCESS;
            break;
        case SDL_EVENT_MOUSE_MOTION: {
            event_mouse_motion(window, event, true);
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            event_mouse_button_down(window, event);
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            event_mouse_button_up(window, event);
        }
        default:
            break;
    };
}
