


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
#include "game.h"
#include "clay_renderer_sdl3.h"
#include "clay_sandbox_ui.h"

static const Uint32 FONT_ID = 0;

static const Clay_Color COLOR_ORANGE    = (Clay_Color) {225, 138, 50, 255};
static const Clay_Color COLOR_BLUE      = (Clay_Color) {111, 173, 162, 255};
static const Clay_Color COLOR_LIGHT     = (Clay_Color) {224, 215, 210, 255};

#define RESIZE_BORDER_THICKNESS 8
#define DRAG_HANDLE_HEIGHT 30

#define RESIZE_TOP (1 << 0)
#define RESIZE_BOTTOM (1 << 1)
#define RESIZE_LEFT (1 << 2)
#define RESIZE_RIGHT (1 << 3)

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

static int get_resize_edges(int mouse_x, int mouse_y, int win_w, int win_h) {
    int edges = 0;
    if (mouse_x < RESIZE_BORDER_THICKNESS) {
        edges |= RESIZE_LEFT;
    }
    if (mouse_x >= win_w - RESIZE_BORDER_THICKNESS) {
        edges |= RESIZE_RIGHT;
    }
    if (mouse_y < RESIZE_BORDER_THICKNESS) {
        edges |= RESIZE_TOP;
    }
    if (mouse_y >= win_h - RESIZE_BORDER_THICKNESS) {
        edges |= RESIZE_BOTTOM;
    }
    return edges;
}

Uint32 system_timer_long_poll_callback(void *user_data, SDL_TimerID timer_id, uint32_t interval) {
    app_state_t *app_state = (app_state_t *) user_data;
    snprintf(app_state->sandbox_ui_data.fps_text_data, 32, "fps: %3.2f", app_state->fps);
    app_state->sandbox_ui_data.fps_text = (Clay_String) { 
        .length = strlen(app_state->sandbox_ui_data.fps_text_data), .chars = app_state->sandbox_ui_data.fps_text_data 
    };
    return(interval);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    if (!TTF_Init()) {
        return SDL_APP_FAILURE;
    }

    app_state_t *state = SDL_calloc(1, sizeof(app_state_t));
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
        &state->window, 
        &state->rendererData.renderer)
    ) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetWindowResizable(state->window, true);
    SDL_SetWindowBordered(state->window, false);
    state->is_dragging = false;
    state->is_resizing = false;
    state->resize_edges = 0;
    state->cursor_arrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    state->cursor_ns_resize = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NS_RESIZE);
    state->cursor_ew_resize = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_EW_RESIZE);
    state->cursor_nwse_resize = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NWSE_RESIZE);
    state->cursor_nesw_resize = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NESW_RESIZE);

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
    SDL_GetWindowSize(state->window, &width, &height);
    Clay_Initialize(clayMemory, (Clay_Dimensions) { (float) width, (float) height }, (Clay_ErrorHandler) { HandleClayErrors });
    Clay_SetMeasureTextFunction(SDL_MeasureText, state->rendererData.fonts);

    state->sandbox_ui_data = sandbox_data_initialize();

    int w;
    int h;
    SDL_GetWindowSize(state->window, &w, &h);
    state->sandbox_ui_data.game_texture = SDL_CreateTexture(
        state->rendererData.renderer, SDL_PIXELFORMAT_RGBA8888, 
        SDL_TEXTUREACCESS_TARGET, w, h
    );
    if (!state->sandbox_ui_data.game_texture) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create game texture: %s", SDL_GetError());
        return(SDL_APP_FAILURE);
    }

    game_init(&state->game_state);
    state->last_game_update_time = SDL_GetTicksNS();

    //Clay_SetDebugModeEnabled(true);
    // turn on vsync to limit fps for now, TODO: something smarter later
    SDL_SetRenderVSync(state->rendererData.renderer, 1);

    *appstate = state;
    return SDL_APP_CONTINUE;
}

static void event_mouse_button_down(app_state_t *state, SDL_Event *event) {
    if (event->button.button == SDL_BUTTON_LEFT) {
        int win_w;
        int win_h;
        SDL_GetWindowSize(state->window, &win_w, &win_h);
        int edges = get_resize_edges(event->button.x, event->button.y, win_w, win_h);

        float global_mouse_x;
        float global_mouse_y;
        SDL_GetGlobalMouseState(&global_mouse_x, &global_mouse_y);
        state->action_start_mouse_pos.x = (int) global_mouse_x;
        state->action_start_mouse_pos.y = (int) global_mouse_y;

        SDL_GetWindowPosition(state->window, &state->action_start_window_rect.x, &state->action_start_window_rect.y);
        SDL_GetWindowSize(state->window, &state->action_start_window_rect.w, &state->action_start_window_rect.h);

        if (edges != 0) {
            state->is_resizing = true;
            state->resize_edges = edges;
        } else if (event->button.y < DRAG_HANDLE_HEIGHT) {
            state->is_dragging = true;
        }
    }
}

static void event_mouse_button_up(app_state_t *state, SDL_Event *event) {
    if (event->button.button == SDL_BUTTON_LEFT) {
        state->is_dragging = false;
        state->is_resizing = false;
        state->resize_edges = 0;
    }
}

static void event_key_down(app_state_t *state, SDL_Event *event) {
    if (event->key.key == SDLK_ESCAPE) {
        if (state->is_mouse_locked) {
            state->is_mouse_locked = false;
            SDL_SetWindowRelativeMouseMode(state->window, false);
            state->game_state.has_mouse_context = false;
        }
    }
}

static void manage_resize_cursor(app_state_t *state, SDL_Event *event) {
    int win_w;
    int win_h;
    SDL_GetWindowSize(state->window, &win_w, &win_h);
    int edges = get_resize_edges(event->motion.x, event->motion.y, win_w, win_h);
    SDL_Cursor *cursor_to_set = state->cursor_arrow;

    switch (edges) {
        case RESIZE_TOP: cursor_to_set = state->cursor_ns_resize; break;
        case RESIZE_BOTTOM: cursor_to_set = state->cursor_ns_resize; break;
        case RESIZE_LEFT: cursor_to_set = state->cursor_ew_resize; break;
        case RESIZE_RIGHT: cursor_to_set = state->cursor_ew_resize; break;
        case RESIZE_TOP | RESIZE_LEFT: cursor_to_set = state->cursor_nwse_resize; break;
        case RESIZE_BOTTOM | RESIZE_RIGHT: cursor_to_set = state->cursor_nwse_resize; break;
        case RESIZE_TOP | RESIZE_RIGHT: cursor_to_set = state->cursor_nesw_resize; break;
        case RESIZE_BOTTOM | RESIZE_LEFT: cursor_to_set = state->cursor_nesw_resize; break;
        default:
            break;
    }
    SDL_SetCursor(cursor_to_set);
}

static void event_mouse_motion(app_state_t *state, SDL_Event *event) {
    if (state->is_dragging) {
        float global_mouse_x;
        float global_mouse_y;
        SDL_GetGlobalMouseState(&global_mouse_x, &global_mouse_y);
        int delta_x = (int)global_mouse_x - state->action_start_mouse_pos.x;
        int delta_y = (int)global_mouse_y - state->action_start_mouse_pos.y;

        int new_x = state->action_start_window_rect.x + delta_x;
        int new_y = state->action_start_window_rect.y + delta_y;

        SDL_SetWindowPosition(state->window, new_x, new_y);
    } else if (state->is_resizing) {
        float global_mouse_x;
        float global_mouse_y;
        SDL_GetGlobalMouseState(&global_mouse_x, &global_mouse_y);
        
        int delta_x = (int)global_mouse_x - state->action_start_mouse_pos.x;
        int delta_y = (int)global_mouse_y - state->action_start_mouse_pos.y;

        int new_x = state->action_start_window_rect.x;
        int new_y = state->action_start_window_rect.y;
        int new_w = state->action_start_window_rect.w;
        int new_h = state->action_start_window_rect.h;

        if (state->resize_edges & RESIZE_LEFT) {
            new_x += delta_x;
            new_w -= delta_x;
        }
        if (state->resize_edges & RESIZE_RIGHT) {
            new_w += delta_x;
        }
        if (state->resize_edges & RESIZE_TOP) {
            new_y += delta_y;
            new_h -= delta_y;
        }
        if (state->resize_edges & RESIZE_BOTTOM) {
            new_h += delta_y;
        }

        SDL_SetWindowPosition(state->window, new_x, new_y);
        SDL_SetWindowSize(state->window, new_w, new_h);
    } else {
        manage_resize_cursor(state, event);
    }
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    app_state_t *state = appstate;
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

    // if (state->sandbox_ui_data.should_quit) {
    //     printf("SDL_AppEvent: should_quit: %d\n", state->sandbox_ui_data.should_quit);
    //     return(SDL_APP_SUCCESS);
    // }

    // game events
    game_handle_event(&state->game_state, event);

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

void calculate_fps(app_state_t *state) {
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
    app_state_t *state = appstate;
    state->fps_frame_counter += 1;
    calculate_fps(state);

    uint64_t now = SDL_GetTicksNS();
    uint64_t dt_ns = now - state->last_game_update_time;
    game_update(&state->game_state, dt_ns);
    state->last_game_update_time = now;

    SDL_SetRenderTarget(state->rendererData.renderer, state->sandbox_ui_data.game_texture);
    game_draw(&state->game_state, state->rendererData.renderer, state->sandbox_ui_data.game_texture);
    SDL_SetRenderTarget(state->rendererData.renderer, NULL);

    Clay_RenderCommandArray render_commands = sandbox_create_layout(state);

    SDL_SetRenderDrawColor(state->rendererData.renderer, 0, 0, 0, 255);
    SDL_RenderClear(state->rendererData.renderer);

    SDL_Clay_RenderClayCommands(&state->rendererData, &render_commands);

    SDL_RenderPresent(state->rendererData.renderer);

    if (state->sandbox_ui_data.should_quit) {
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

    app_state_t *state = appstate;

    if (sample_image) {
        SDL_DestroyTexture(sample_image);
    }

    if (state->sandbox_ui_data.game_texture) {
        SDL_DestroyTexture(state->sandbox_ui_data.game_texture);
    }

    if (state) {
        if (state->rendererData.renderer)
            SDL_DestroyRenderer(state->rendererData.renderer);

        if (state->window)
            SDL_DestroyWindow(state->window);

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
    app_state_t *state = NULL;
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
        if (state->sandbox_ui_data.should_quit) {
            quit = true;
        }
        //sleep?
    }
    SDL_AppResult quit_result = SDL_APP_SUCCESS;
    SDL_AppQuit((void*)state, quit_result);
}
