


/*
 * Currently using a "main" and not the callback approach. Comment out "main" and 
 * uncomment "SDL_MAIN_USE_CALLBACKS" if you want to go back to the callback driven 
 * approach.
 */
//#define SDL_MAIN_USE_CALLBACKS

#include <stdio.h>

#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#define CLAY_IMPLEMENTATION
#include "clay.h"

#include "app.h"
#include "app_window.h"
#include "clay_renderer_sdl3.h"
#include "bterm_ui.h"

static const Uint32 FONT_ID = 0;

static const Clay_Color COLOR_ORANGE    = (Clay_Color) {225, 138, 50, 255};
static const Clay_Color COLOR_BLUE      = (Clay_Color) {111, 173, 162, 255};
static const Clay_Color COLOR_LIGHT     = (Clay_Color) {224, 215, 210, 255};

SDL_Texture *sample_image;

static inline Clay_Dimensions SDL_MeasureText(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData) {
    TTF_Font **fonts = userData;
    TTF_Font *font = fonts[config->fontId];
    int width, height;

    TTF_SetFontSize(font, config->fontSize);
    if (!TTF_GetStringSize(font, text.chars, text.length, &width, &height)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to measure text: %s", SDL_GetError());
    }

    return (Clay_Dimensions) { (float) width, (float) height };
}

void HandleClayErrors(Clay_ErrorData errorData) {
    printf("%s", errorData.errorText.chars);
}

Clay_RenderCommandArray ClayImageSample_CreateLayout() {
    Clay_BeginLayout();

    Clay_Sizing layoutExpand = {
        .width = CLAY_SIZING_GROW(0),
        .height = CLAY_SIZING_GROW(0)
    };

    CLAY(CLAY_ID("OuterContainer"), {
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .sizing = layoutExpand,
            .padding = CLAY_PADDING_ALL(16),
            .childGap = 16
        }
    }) {
        CLAY(CLAY_ID("SampleImage"), {
            .layout = {
                .sizing = layoutExpand
            },
            .aspectRatio = { 23.0 / 42.0 },
            .image = {
                .imageData = sample_image,
            }
        });
    }

    return Clay_EndLayout();
}

Uint32 system_timer_long_poll_callback(void *user_data, SDL_TimerID timer_id, uint32_t interval) {
    app_t *app_state = (app_t *) user_data;
    snprintf(app_state->ui_data.fps_text_data, 32, "fps: %3.2f", app_state->fps);
    app_state->ui_data.fps_text = (Clay_String) { 
        .length = strlen(app_state->ui_data.fps_text_data), .chars = app_state->ui_data.fps_text_data 
    };
    return(interval);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    if (!TTF_Init()) {
        return SDL_APP_FAILURE;
    }

    app_t *state = SDL_calloc(1, sizeof(app_t));
    if (!state) {
        return SDL_APP_FAILURE;
    }
    *appstate = state;

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

    /* Initialize Clay */
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

    //Clay_SetDebugModeEnabled(true);
    // turn on vsync to limit fps for now, TODO: something smarter later
    SDL_SetRenderVSync(state->rendererData.renderer, 1);

    *appstate = state;
    return SDL_APP_CONTINUE;
}

static void event_mouse_button_down(app_t *app, SDL_Event *event) {
    // app mouse down!
    if (event->button.button == SDL_BUTTON_LEFT) {
        // TODO: is on panel boundary?
    }
}

static void event_mouse_button_up(app_t *state, SDL_Event *event) {
    if (event->button.button == SDL_BUTTON_LEFT) {
        state->is_resizing_pane = false;
    }
}

static void event_key_down(app_t *state, SDL_Event *event) {
    if (event->key.key == SDLK_ESCAPE) {
        if (state->is_mouse_locked) {
            state->is_mouse_locked = false;
            SDL_SetWindowRelativeMouseMode(state->window.sdl_window, false);
        }
    }
}

static void event_mouse_motion(app_t *state, SDL_Event *event) {
    if (state->is_resizing_pane) {
        float global_mouse_x;
        float global_mouse_y;
        SDL_GetGlobalMouseState(&global_mouse_x, &global_mouse_y);
        
        int delta_x = (int)global_mouse_x - state->action_start_panel_rect.x;
        state->ui_data.sidebar_width = state->action_start_pane_width + delta_x;

        // Add constraints
        int win_w, win_h;
        SDL_GetWindowSize(state->window.sdl_window, &win_w, &win_h);
        if (state->ui_data.sidebar_width < 100) { // Min width
            state->ui_data.sidebar_width = 100;
        }
        if (state->ui_data.sidebar_width > win_w - 100) { // Max width (leave 100px for right pane)
            state->ui_data.sidebar_width = win_w - 100;
        }
    }
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    app_t *state = appstate;
    SDL_AppResult ret_val = SDL_APP_CONTINUE;

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

    // handle window events, like resizing or dragging
    app_window_sdl_event(&state->window, event);

    // if (state->sandbox_ui_data.should_quit) {
    //     printf("SDL_AppEvent: should_quit: %d\n", state->sandbox_ui_data.should_quit);
    //     return(SDL_APP_SUCCESS);
    // }

    // Process app specific events
    switch (event->type) {
        case SDL_EVENT_QUIT:
            ret_val = SDL_APP_SUCCESS;
            break;
        case SDL_EVENT_MOUSE_MOTION: {
            event_mouse_motion(state, event);
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            event_mouse_button_down(state, event);
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            event_mouse_button_up(state, event);
        }
        case SDL_EVENT_KEY_DOWN: {
            event_key_down(state, event);
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
        printf("fps: %f\n", state->fps);
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
