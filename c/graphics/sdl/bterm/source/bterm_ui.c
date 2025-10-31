
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>
#include "clay.h"

#include "bterm_ui.h"


static const int FONT_ID_BODY_16 = 0;
static Clay_Color COLOR_WHITE = { 255, 255, 255, 255};
static Clay_Color COLOR_NEAR_BLACK = { 20, 20, 20, 255};
static Clay_Color COLOR_DARK_GREY = { 40, 40, 40, 255};
static Clay_Color COLOR_MED_GREY = { 60, 60, 60, 255};
static Clay_Color COLOR_LIGHT_GREY = { 200, 200, 200, 255};

typedef enum {
    MAIN_CONTENT_NONE,
    MAIN_CONTENT_FLY
} main_content_t;

ui_data_t ui_data_initialize() {
    char* memory = malloc(1024);
    if (memory == NULL) {
        printf("ui_data_initialize failed to malloc!\n");
    }

    ui_data_t data = {
        .frameArena = { .memory = (char*) memory },
        .should_quit = false,
        .fps_text_data = { 0 },
        .fps_text = {
            .chars = data.fps_text_data,
            .isStaticallyAllocated = true,
            .length = 2
        },
        .sidebar_width = 120
    };
    data.fps_text = (Clay_String) { .length = 0, .chars = data.fps_text_data };
    return(data);
}

typedef struct {
    int32_t requested_index;
    int32_t* selected_index;
} sidebar_click_data_t;

static void RenderDropdownMenuItem(Clay_String text) {
    CLAY_AUTO_ID({.layout = { .padding = CLAY_PADDING_ALL(16)}}) {
        CLAY_TEXT(text, CLAY_TEXT_CONFIG({
            .fontId = FONT_ID_BODY_16,
            .fontSize = 16,
            .textColor = { 255, 255, 255, 255 }
        }));
    }
}

static void HandleSidebarInteraction(
    Clay_ElementId elementId,
    Clay_PointerData pointerData,
    intptr_t userData
) {
    // sidebar_click_data_t *clickData = (sidebar_click_data_t*)userData;
    // if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    //     if (clickData->requested_index >= 0 && clickData->requested_index < documents.length) {
    //         *clickData->selected_index = clickData->requested_index;
    //     }
    // }
}

static void handle_close_button_interaction(
    Clay_ElementId element_id,
    Clay_PointerData pointer_data,
    intptr_t user_data
) {
    ui_data_t *data = (ui_data_t*)user_data;
    if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        //SDL_Event quit_event;
        //quit_event.type = SDL_EVENT_QUIT;
        //SDL_PushEvent(&quit_event); 
        //SDL_Quit();     
        data->should_quit = true;
    }
}

static void handle_pane_resizer_interaction(
    Clay_ElementId element_id,
    Clay_PointerData pointer_data,
    intptr_t user_data
) {
    app_t *state = (app_t *)user_data;
    state->is_hovering_pane_resizer = true; // Flag for cursor management in main.c
    SDL_SetCursor(state->cursors.ew_resize); // Set resize cursor

    if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        state->is_resizing_pane = true;
        state->action_start_pane_width = state->ui_data.sidebar_width;

        // Store global mouse pos, not local
        float global_x, global_y;
        SDL_GetGlobalMouseState(&global_x, &global_y);
        state->action_start_panel_rect.x = (int)global_x;
        state->action_start_panel_rect.y = (int)global_y;
    }
}

Clay_RenderCommandArray ui_create_layout(app_t *app) {
    ui_data_t *data = &app->ui_data;
    data->frameArena.offset = 0;

    Clay_BeginLayout();

    Clay_Sizing layoutExpand = {
        .width = CLAY_SIZING_GROW(0),
        .height = CLAY_SIZING_GROW(0)
    };
    Clay_Color contentBackgroundColor = { 90, 90, 90, 255 };

    CLAY(CLAY_ID("OuterContainer"), {
        .backgroundColor = { 40, 40, 40, 255 },
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .sizing = layoutExpand,
            .padding = CLAY_PADDING_ALL(2),
            .childGap = 2
        }
    }) {
        // window children here:
        CLAY(CLAY_ID("HeaderBar"), {
            .layout = {
                .sizing = {
                    .height = CLAY_SIZING_FIXED(30),
                    .width = CLAY_SIZING_GROW(0)
                },
                .padding = { 4, 4, 0, 0 },
                .childGap = 4,
                .childAlignment = {
                    .y = CLAY_ALIGN_Y_CENTER
                }
            },
            .backgroundColor = { 200, 200, 200, 255 },
            .cornerRadius = CLAY_CORNER_RADIUS(2)
        }) {
            // header bar children here:
            CLAY(CLAY_ID("FileButton"), {
                .layout = { .padding = { 8, 8, 2, 2 }},
                .backgroundColor = {140, 140, 140, 255 },
                .cornerRadius = CLAY_CORNER_RADIUS(2)
            }) {
                CLAY_TEXT(CLAY_STRING("File"), CLAY_TEXT_CONFIG({
                    .fontId = FONT_ID_BODY_16,
                    .fontSize = 16,
                    .textColor = { 255, 255, 255, 255 }
                }));

                bool fileMenuVisible =
                    Clay_PointerOver(Clay_GetElementId(CLAY_STRING("FileButton")))
                    ||
                    Clay_PointerOver(Clay_GetElementId(CLAY_STRING("FileMenu")));
                
                if (fileMenuVisible) { // Below has been changed slightly to fix the small bug where the menu would dismiss when mousing over the top gap
                    CLAY(CLAY_ID("FileMenu"), {
                        .floating = {
                            .attachTo = CLAY_ATTACH_TO_PARENT,
                            .attachPoints = {
                                .parent = CLAY_ATTACH_POINT_LEFT_BOTTOM
                            },
                        },
                        .layout = {
                            .padding = {0, 0, 8, 8 }
                        }
                    }) {
                        CLAY_AUTO_ID({
                            .layout = {
                                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                .sizing = {
                                    .width = CLAY_SIZING_FIXED(200)
                                },
                            },
                            .backgroundColor = {40, 40, 40, 255 },
                            .cornerRadius = CLAY_CORNER_RADIUS(8)
                        }) {
                            // Render dropdown items here
                            //RenderDropdownMenuItem(CLAY_STRING("New"));
                            //RenderDropdownMenuItem(CLAY_STRING("Open"));
                            RenderDropdownMenuItem(CLAY_STRING("Close"));
                        }
                    }
                }
            }
            CLAY(CLAY_ID("fps"), {
                .layout = { .padding = { 8, 8, 2, 2 }},
            }) {
                CLAY_TEXT(data->fps_text, CLAY_TEXT_CONFIG({
                    .fontId = FONT_ID_BODY_16,
                    .fontSize = 16,
                    .textColor = { 255, 255, 255, 255 }
                }));
            }

            CLAY_AUTO_ID({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) } } }) {}

            CLAY(CLAY_ID("CloseButton"), {
                .layout = { .padding = { 8, 8, 2, 2 }},
                .backgroundColor = Clay_Hovered() ? (Clay_Color){160, 160, 160, 255} : (Clay_Color){140, 140, 140, 255},
                .cornerRadius = CLAY_CORNER_RADIUS(2)
            }) {
                Clay_OnHover(handle_close_button_interaction, (intptr_t) data);
                CLAY_TEXT(CLAY_STRING("x"), CLAY_TEXT_CONFIG({
                    .fontId = FONT_ID_BODY_16,
                    .fontSize = 16,
                    .textColor =  Clay_Hovered() ? (Clay_Color){255, 50, 50, 255} : (Clay_Color){ 255, 255, 255, 255 }
                }));
            }
        }
        // everything below the file bar
        CLAY(CLAY_ID("LowerContent"), {
            .layout = { .sizing = layoutExpand, .childGap = 8 }
        }) {
            CLAY(CLAY_ID("Sidebar"), {
                .backgroundColor = contentBackgroundColor,
                .layout = {
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .padding = CLAY_PADDING_ALL(16),
                    .childGap = 8,
                    .sizing = {
                        .width = CLAY_SIZING_FIXED(data->sidebar_width),
                        .height = CLAY_SIZING_GROW(0)
                    }
                }
            }) {
                // not sure where i want to put the fps yet...
                CLAY(CLAY_ID("fps2"), {
                    .layout = { .padding = { 8, 8, 2, 2 }},
                }) {
                    CLAY_TEXT(data->fps_text, CLAY_TEXT_CONFIG({
                        .fontId = FONT_ID_BODY_16,
                        .fontSize = 16,
                        .textColor = { 255, 255, 255, 255 }
                    }));
                }
                CLAY_TEXT(CLAY_STRING("- /home"), CLAY_TEXT_CONFIG({
                        .fontId = FONT_ID_BODY_16,
                        .fontSize = 16,
                        .textColor = { 255, 255, 255, 255 }
                    }));
                CLAY_TEXT(CLAY_STRING("-   /user"), CLAY_TEXT_CONFIG({
                        .fontId = FONT_ID_BODY_16,
                        .fontSize = 16,
                        .textColor = { 255, 255, 255, 255 }
                    }));
                CLAY_TEXT(CLAY_STRING("-     /file1.txt"), CLAY_TEXT_CONFIG({
                        .fontId = FONT_ID_BODY_16,
                        .fontSize = 16,
                        .textColor = { 255, 255, 255, 255 }
                    }));
            }
            CLAY(CLAY_ID("PaneResizer"), {
                .backgroundColor = Clay_Hovered() ? COLOR_LIGHT_GREY : COLOR_DARK_GREY,
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_FIXED(4), // Resizer is 4px wide
                        .height = CLAY_SIZING_GROW(0)
                    }
                }
            }) {
                Clay_OnHover(handle_pane_resizer_interaction, (intptr_t)app);
            }
            CLAY(CLAY_ID("MainContent"), {
                .backgroundColor = { 20, 20, 20, 255 }, // Dark background for the game
                .layout = {
                    .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) },
                }
            }) {
                // Clay_OnHover(handle_game_area_interaction, (intptr_t)app_state);
                // CLAY_AUTO_ID({
                //     .layout = { .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) } },
                //     .image = { .imageData = data->game_texture }
                // });
            }
        }
    }

    Clay_RenderCommandArray renderCommands = Clay_EndLayout();
    for (int32_t i = 0; i < renderCommands.length; i++) {
        Clay_RenderCommandArray_Get(&renderCommands, i)->boundingBox.y += data->yOffset;
    }

    return renderCommands;
}
