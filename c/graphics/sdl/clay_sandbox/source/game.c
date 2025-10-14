// from sdl3 example: woodeneye-008

#include "game.h"
#include <SDL3/SDL.h>

#define CIRCLE_DRAW_SIDES 32
#define CIRCLE_DRAW_SIDES_LEN (CIRCLE_DRAW_SIDES + 1)

static void initPlayer(player_t *player) {
    player->pos[0] = 8.0 * (0 & 1 ? -1.0 : 1.0);
    player->pos[1] = 0;
    player->pos[2] = 8.0 * (0 & 1 ? -1.0 : 1.0) * (0 & 2 ? -1.0 : 1.0);
    player->vel[0] = 0;
    player->vel[1] = 0;
    player->vel[2] = 0;
    player->yaw = 0x20000000 + (0 & 1 ? 0x80000000 : 0) + (0 & 2 ? 0x40000000 : 0);
    player->pitch = -0x08000000;
    player->radius = 0.5f;
    player->height = 1.5f;
    player->wasd = 0;
    player->mouse = 0;
    player->keyboard = 0;
}

static void initEdges(int scale, float (*edges)[6], int edges_len) {
    const float r = (float)scale;
    const int map[24] = { 0,1, 1,3, 3,2, 2,0, 7,6, 6,4, 4,5, 5,7, 6,2, 3,7, 0,4, 5,1 };
    for(int i = 0; i < 12; i++) {
        for (int j = 0; j < 3; j++) {
            edges[i][j+0] = (map[i*2+0] & (1 << j) ? r : -r);
            edges[i][j+3] = (map[i*2+1] & (1 << j) ? r : -r);
        }
    }
    for(int i = 0; i < scale; i++) {
        float d = (float)(i * 2);
        for (int j = 0; j < 2; j++) {
            edges[i+12][3*j+0] = j ? r : -r;
            edges[i+12][3*j+1] = -r;
            edges[i+12][3*j+2] = d-r;
            edges[i+12+scale][3*j+0] = d-r;
            edges[i+12+scale][3*j+1] = -r;
            edges[i+12+scale][3*j+2] = j ? r : -r;
        }
    }
}

static void drawCircle(SDL_Renderer *renderer, float r, float x, float y) {
    SDL_FPoint points[CIRCLE_DRAW_SIDES_LEN];
    for (int i = 0; i < CIRCLE_DRAW_SIDES_LEN; i++) {
        float ang = 2.0f * SDL_PI_F * (float)i / (float)CIRCLE_DRAW_SIDES;
        points[i].x = x + r * SDL_cosf(ang);
        points[i].y = y + r * SDL_sinf(ang);
    }
    SDL_RenderLines(renderer, (const SDL_FPoint*)&points, CIRCLE_DRAW_SIDES_LEN);
}

static void drawClippedSegment(SDL_Renderer *renderer, float ax, float ay, float az, float bx, float by, float bz, float x, float y, float z, float w) {
    if (az >= -w && bz >= -w) return;
    float dx = ax - bx;
    float dy = ay - by;
    if (az > -w) {
        float t = (-w - bz) / (az - bz);
        ax = bx + dx * t;
        ay = by + dy * t;
        az = -w;
    } else if (bz > -w) {
        float t = (-w - az) / (bz - az);
        bx = ax - dx * t;
        by = ay - dy * t;
        bz = -w;
    }
    ax = -z * ax / az;
    ay = -z * ay / az;
    bx = -z * bx / bz;
    by = -z * by / bz;
    SDL_RenderLine(renderer, x + ax, y - ay, x + bx, y - by);
}

// --- Public functions ---

void game_init(game_state_t *state) {
    state->player_count = 1;
    initPlayer(&state->player);
    initEdges(MAP_BOX_SCALE, state->edges, MAP_BOX_EDGES_LEN);
    state->debug_string[0] = '\0';
    state->has_mouse_context = false;
}

void game_handle_event(game_state_t *state, SDL_Event *event) {
    player_t *player = &state->player;
    int player_count = state->player_count;
    switch (event->type) {
        case SDL_EVENT_MOUSE_MOTION: {
            if (state->has_mouse_context) {
                state->player.yaw -= ((int)event->motion.xrel) * 0x00080000;
                state->player.pitch = SDL_max(
                    -0x40000000, SDL_min(
                        0x40000000, state->player.pitch - ((int)event->motion.yrel) * 0x00080000
                    )
                );
            }
            break;
        }
        case SDL_EVENT_KEY_DOWN: {
            if (state->has_mouse_context) {
                if (event->key.key == SDLK_W) state->player.wasd |= 1;
                if (event->key.key == SDLK_A) state->player.wasd |= 2;
                if (event->key.key == SDLK_S) state->player.wasd |= 4;
                if (event->key.key == SDLK_D) state->player.wasd |= 8;
                if (event->key.key == SDLK_SPACE) state->player.wasd |= 16;
            }
            break;
        }
        case SDL_EVENT_KEY_UP: {
            if (state->has_mouse_context) {
                if (event->key.key == SDLK_W) state->player.wasd &= ~1;
                if (event->key.key == SDLK_A) state->player.wasd &= ~2;
                if (event->key.key == SDLK_S) state->player.wasd &= ~4;
                if (event->key.key == SDLK_D) state->player.wasd &= ~8;
                if (event->key.key == SDLK_SPACE) state->player.wasd &= ~16;
            }
            break;
        }
    }
}

void game_update(game_state_t *state, Uint64 dt_ns) {
    player_t *player = &state->player;
    double rate = 6.0;
    double time = (double)dt_ns * 1e-9;
    double drag = SDL_exp(-time * rate);
    double diff = 1.0 - drag;
    double mult = 60.0;
    double grav = 25.0;
    double yaw = (double)player->yaw;
    double rad = yaw * SDL_PI_D / 2147483648.0;
    double cos = SDL_cos(rad);
    double sin = SDL_sin(rad);
    unsigned char wasd = player->wasd;
    double dirX = (wasd & 8 ? 1.0 : 0.0) - (wasd & 2 ? 1.0 : 0.0);
    double dirZ = (wasd & 4 ? 1.0 : 0.0) - (wasd & 1 ? 1.0 : 0.0);
    double norm = dirX * dirX + dirZ * dirZ;
    double accX = mult * (norm == 0 ? 0 : ( cos*dirX + sin*dirZ) / SDL_sqrt(norm));
    double accZ = mult * (norm == 0 ? 0 : (-sin*dirX + cos*dirZ) / SDL_sqrt(norm));
    double velX = player->vel[0];
    double velY = player->vel[1];
    double velZ = player->vel[2];
    player->vel[0] -= velX * diff;
    player->vel[1] -= grav * time;
    player->vel[2] -= velZ * diff;
    player->vel[0] += diff * accX / rate;
    player->vel[2] += diff * accZ / rate;
    player->pos[0] += (time - diff/rate) * accX / rate + diff * velX / rate;
    player->pos[1] += -0.5 * grav * time * time + velY * time;
    player->pos[2] += (time - diff/rate) * accZ / rate + diff * velZ / rate;
    double scale = (double)MAP_BOX_SCALE;
    double bound = scale - player->radius;
    double posX = SDL_max(SDL_min(bound, player->pos[0]), -bound);
    double posY = SDL_max(SDL_min(bound, player->pos[1]), player->height - scale);
    double posZ = SDL_max(SDL_min(bound, player->pos[2]), -bound);
    if (player->pos[0] != posX) player->vel[0] = 0;
    if (player->pos[1] != posY) player->vel[1] = (wasd & 16) ? 8.4375 : 0;
    if (player->pos[2] != posZ) player->vel[2] = 0;
    player->pos[0] = posX;
    player->pos[1] = posY;
    player->pos[2] = posZ;
}

void game_draw(game_state_t *state, SDL_Renderer *renderer, SDL_Texture *target_texture) {
    float w;
    float h;
    //SDL_GetTextureProperties()
    if (!SDL_GetTextureSize(target_texture, &w, &h)) {
        return;
    }
    // if (SDL_GetRenderOutputSize(renderer, &w, &h) != 0) {
    //     return;
    // }
    //printf("%s: renderer output size: %d, %d\n", );

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    if (state->player_count > 0) {
        float wf = (float)w;
        float hf = (float)h;

        const player_t *player = &state->player;
        float hor_origin = wf * 0.5f;
        float ver_origin = hf * 0.5f;
        float cam_origin = (float)(0.5 * SDL_sqrt(wf * wf + hf * hf));

        double x0 = player->pos[0];
        double y0 = player->pos[1];
        double z0 = player->pos[2];
        double bin_rad = SDL_PI_D / 2147483648.0;
        double yaw_rad   = bin_rad * player->yaw;
        double pitch_rad = bin_rad * player->pitch;
        double cos_yaw   = SDL_cos(yaw_rad);
        double sin_yaw   = SDL_sin(yaw_rad);
        double cos_pitch = SDL_cos(pitch_rad);
        double sin_pitch = SDL_sin(pitch_rad);
        double mat[9] = {
            cos_yaw          ,          0, -sin_yaw          ,
            sin_yaw*sin_pitch,  cos_pitch,  cos_yaw*sin_pitch,
            sin_yaw*cos_pitch, -sin_pitch,  cos_yaw*cos_pitch
        };

        SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);
        for (int k = 0; k < MAP_BOX_EDGES_LEN; k++) {
            const float *line = state->edges[k];
            float ax = (float)(mat[0] * (line[0] - x0) + mat[1] * (line[1] - y0) + mat[2] * (line[2] - z0));
            float ay = (float)(mat[3] * (line[0] - x0) + mat[4] * (line[1] - y0) + mat[5] * (line[2] - z0));
            float az = (float)(mat[6] * (line[0] - x0) + mat[7] * (line[1] - y0) + mat[8] * (line[2] - z0));
            float bx = (float)(mat[0] * (line[3] - x0) + mat[1] * (line[4] - y0) + mat[2] * (line[5] - z0));
            float by = (float)(mat[3] * (line[3] - x0) + mat[4] * (line[4] - y0) + mat[5] * (line[5] - z0));
            float bz = (float)(mat[6] * (line[3] - x0) + mat[7] * (line[4] - y0) + mat[8] * (line[5] - z0));
            drawClippedSegment(renderer, ax, ay, az, bx, by, bz, hor_origin, ver_origin, cam_origin, 1);
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderLine(renderer, hor_origin, ver_origin-10, hor_origin, ver_origin+10);
        SDL_RenderLine(renderer, hor_origin-10, ver_origin, hor_origin+10, ver_origin);
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(renderer, 0, 0, state->debug_string);
}
