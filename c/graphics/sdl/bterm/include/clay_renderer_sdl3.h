#ifndef CLAY_RENDERER_SDL3_H
#define CLAY_RENDERER_SDL3_H

#include "./clay.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>

typedef struct {
    SDL_Renderer *renderer;
    TTF_TextEngine *textEngine;
    TTF_Font **fonts;
} Clay_SDL3RendererData;

/* Global for convenience. Even in 4K this is enough for smooth curves (low radius or rect size 
 * coupled with no AA or low resolution might make it appear as jagged curves) */
static int NUM_CIRCLE_SEGMENTS = 16;

void SDL_Clay_RenderFillRoundedRect(Clay_SDL3RendererData *rendererData, const SDL_FRect rect, const float cornerRadius, const Clay_Color _color);
void SDL_Clay_RenderArc(Clay_SDL3RendererData *rendererData, const SDL_FPoint center, const float radius, const float startAngle, const float endAngle, const float thickness, const Clay_Color color);
void SDL_Clay_RenderClayCommands(Clay_SDL3RendererData *rendererData, Clay_RenderCommandArray *rcommands);

#endif // CLAY_RENDERER_SDL3_H
