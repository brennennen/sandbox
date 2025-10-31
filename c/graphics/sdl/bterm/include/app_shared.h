#ifndef APP_SHARED_H
#define APP_SHARED_H

#include <SDL3/SDL.h>

typedef struct {
    SDL_Cursor *arrow;
    SDL_Cursor *ns_resize;
    SDL_Cursor *ew_resize;
    SDL_Cursor *nwse_resize;
    SDL_Cursor *nesw_resize;
} app_cursors_t;

#endif // APP_SHARED_H