
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>
#include "clay.h"

#include "clay_sandbox_ui.h"



static const int FONT_ID_BODY_16 = 0;
static Clay_Color COLOR_WHITE = { 255, 255, 255, 255};

typedef enum {
    MAIN_CONTENT_NONE,
    MAIN_CONTENT_FLY
} main_content_t;

clay_sandbox_ui_data_t sandbox_data_initialize() {
    char* memory = malloc(1024);
    if (memory == NULL) {
        printf("sandbox_data_initialize failed to malloc!\n");
    }

    clay_sandbox_ui_data_t data = {
        .frameArena = { .memory = (char*) memory },
        .should_quit = false,
        .fps_text_data = { 0 },
        .fps_text = {
            .chars = data.fps_text_data,
            .isStaticallyAllocated = true,
            .length = 2
        }
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
    clay_sandbox_ui_data_t *data = (clay_sandbox_ui_data_t*)user_data;
    if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        //SDL_Event quit_event;
        //quit_event.type = SDL_EVENT_QUIT;
        //SDL_PushEvent(&quit_event); 
        //SDL_Quit();     
        data->should_quit = true;
    }
}

static void handle_game_area_interaction(
    Clay_ElementId element_id,
    Clay_PointerData pointer_data,
    intptr_t user_data
) {
    app_state_t *state = (app_state_t *)user_data;
    if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        printf("%s: clicked inside main content\n", __func__);
        if (!state->is_mouse_locked) {
            state->is_mouse_locked = true;
            SDL_SetWindowRelativeMouseMode(state->window, true);
            state->game_state.has_mouse_context = true;
        }
    }
}

Clay_RenderCommandArray sandbox_create_layout(app_state_t *app_state) {
    clay_sandbox_ui_data_t *data = &app_state->sandbox_ui_data;
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
                        .width = CLAY_SIZING_FIXED(120),
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
                // TODO: different games/experiences/demos based on what's selected on the sidebar?
            }
            CLAY(CLAY_ID("MainContent"), {
                .backgroundColor = { 20, 20, 20, 255 }, // Dark background for the game
                .layout = {
                    .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) },
                }
            }) {
                Clay_OnHover(handle_game_area_interaction, (intptr_t)app_state);
                CLAY_AUTO_ID({
                    .layout = { .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) } },
                    .image = { .imageData = data->game_texture }
                });
            }
        }
    }

    Clay_RenderCommandArray renderCommands = Clay_EndLayout();
    for (int32_t i = 0; i < renderCommands.length; i++) {
        Clay_RenderCommandArray_Get(&renderCommands, i)->boundingBox.y += data->yOffset;
    }

    return renderCommands;
}
