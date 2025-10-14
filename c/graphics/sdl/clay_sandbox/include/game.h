#ifndef GAME_H
#define GAME_H

#include <SDL3/SDL.h>

#define MAP_BOX_SCALE 16
#define MAP_BOX_EDGES_LEN (12 + MAP_BOX_SCALE * 2)
#define MAX_PLAYER_COUNT 4

typedef struct {
    SDL_MouseID mouse;
    SDL_KeyboardID keyboard;
    double pos[3];
    double vel[3];
    unsigned int yaw;
    int pitch;
    float radius, height;
    unsigned char wasd;
} player_t;

typedef struct {
    int player_count;
    player_t player;
    //player_t players[MAX_PLAYER_COUNT];
    float edges[MAP_BOX_EDGES_LEN][6];
    char debug_string[32];
    bool has_mouse_context;
} game_state_t;

void game_init(game_state_t *state);
void game_handle_event(game_state_t *state, SDL_Event *event);
void game_update(game_state_t *state, Uint64 dt_ns);
void game_draw(game_state_t *state, SDL_Renderer *renderer, SDL_Texture *target_texture);

#endif // GAME_H