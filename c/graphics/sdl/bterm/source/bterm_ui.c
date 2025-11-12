
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
static Clay_Color COLOR_ICON_BLUE = { 100, 150, 255, 255 };

typedef enum {
    MAIN_CONTENT_NONE,
    MAIN_CONTENT_FLY
} main_content_t;


static const Clay_ElementId CLAY_ID_NONE = {0, 0};

// Define the missing ID comparison function
static inline bool Clay_ElementId_Equal(Clay_ElementId id1, Clay_ElementId id2) {
    return id1.baseId == id2.baseId && id1.id == id2.id;
}


Clay_Dimensions SDL_MeasureText(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData) {
    TTF_Font **fonts = userData;
    TTF_Font *font = fonts[config->fontId];
    int width, height;

    TTF_SetFontSize(font, config->fontSize);
    if (!TTF_GetStringSize(font, text.chars, text.length, &width, &height)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to measure text: %s", SDL_GetError());
    }

    return (Clay_Dimensions) { (float) width, (float) height };
}

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

    strncpy(data.connect_string_data, "b@192.168.1.149:22", sizeof(data.connect_string_data) - 1);
    data.connect_string_data[sizeof(data.connect_string_data) - 1] = '\0'; // Ensure null termination
    data.connect_string = (Clay_String) {
        .chars = data.connect_string_data,
        .isStaticallyAllocated = true,
        .length = strlen(data.connect_string_data)
    };

    return(data);
}

static app_t* g_app = NULL;

void set_ui_global_context(app_t* app) {
    g_app = app;
}

app_t* get_ui_global_context() {
    return g_app;
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

static void handle_side_pane_resizer_interaction(
    Clay_ElementId element_id,
    Clay_PointerData pointer_data,
    intptr_t user_data
) {
    app_t *state = (app_t *)user_data;
    state->is_hovering_side_pane_resizer = true;
}

static void handle_connect_button_interaction(
    Clay_ElementId element_id,
    Clay_PointerData pointer_data,
    intptr_t user_data
) {
    app_t *app = (app_t*)user_data;
    app->connect_button_hovering = true;
    if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        SDL_Log("Connect button clicked!");
    }
}

typedef struct {
    app_t* app;
    Clay_ElementId id;
    Clay_String* string_storage;
    size_t buffer_size;
} text_input_click_data_t;

static void handle_text_input_interaction(
    Clay_ElementId element_id,
    Clay_PointerData pointer_data,
    intptr_t user_data
) {
    text_input_click_data_t* data = (text_input_click_data_t*)user_data;
    app_t* app = data->app;

    if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        app->text_input_state.active_id = data->id;
        app->text_input_state.active_string = data->string_storage;
        app->text_input_state.active_buffer_size = data->buffer_size;
        app->text_input_state.cursor_pos = data->string_storage->length;
        app->text_input_state.cursor_visible = true;
        app->text_input_state.cursor_last_blink_time = SDL_GetTicks();
        SDL_StartTextInput(app->window.sdl_window);
    }
}

void Clay_TextInput(
    app_t* app, 
    Clay_ElementId id, 
    Clay_String* string_storage, 
    size_t buffer_size, 
    Clay_TextElementConfig* config
) {
    text_input_click_data_t clickData = {
        .app = app,
        .id = id,
        .string_storage = string_storage,
        .buffer_size = buffer_size
    };

    Clay_OnHover(handle_text_input_interaction, (intptr_t)&clickData);
    
    Clay_Dimensions full_text_dims;
    Clay_StringSlice slice_to_measure;
    full_text_dims = SDL_MeasureText(slice_to_measure, config, app->rendererData.fonts);
    CLAY_TEXT(*string_storage, config);
    if (Clay_ElementId_Equal(app->text_input_state.active_id, id)) {
        if (app->text_input_state.cursor_visible) {
            Clay_Dimensions text_dims_to_cursor = {0, 0};
            if (app->text_input_state.cursor_pos > 0) {
                Clay_StringSlice sub_string = {
                    .chars = string_storage->chars,
                    .length = app->text_input_state.cursor_pos
                };
                text_dims_to_cursor = SDL_MeasureText(sub_string, config, app->rendererData.fonts);
            }
        }
    }
}

static char* build_path_for_node(RemoteFileNode* node) {
    if (node == NULL) {
        return SDL_strdup(".");
    }
    if (node->parent == NULL) {
        return SDL_strdup(node->name);
    }
    char* parent_path = build_path_for_node(node->parent);
    if (parent_path == NULL) {
        return NULL;
    }
    bool is_root_dot = (strcmp(parent_path, ".") == 0);
    size_t len = strlen(parent_path);
    if (is_root_dot) {
        len = 0;
    }
    size_t name_len = strlen(node->name);
    char* full_path = NULL;
    if (is_root_dot) {
        full_path = (char*)SDL_malloc(name_len + 1);
        snprintf(full_path, name_len + 1, "%s", node->name);
    } else {
        full_path = (char*)SDL_malloc(len + 1 + name_len + 1);
        snprintf(full_path, len + 1 + name_len + 1, "%s/%s", parent_path, node->name);
    }
    
    SDL_free(parent_path);
    return full_path;
}

static void handle_treenode_click(
    Clay_ElementId element_id,
    Clay_PointerData pointer_data,
    intptr_t user_data
) {
    if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        RemoteFileNode* node = (RemoteFileNode*)user_data;
        app_t* app = get_ui_global_context(); 
        
        if (node->type != REMOTE_FILE_TYPE_DIRECTORY) return;

        if (node->is_expanded) {
            node->is_expanded = false;
        } else {
            if (node->children) {
                node->is_expanded = true;
            } else {
                if (app->node_awaiting_children) {
                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "UI: Ignoring expand, a request is already pending.");
                    return;
                }
                
                char* path_to_list = build_path_for_node(node);
                SDL_Log("UI: Queuing LIST_DIR for: %s", path_to_list);
                
                app->node_awaiting_children = node;
                
                ssh_request_t* req = create_ssh_request(SSH_COMMAND_LIST_DIR, path_to_list);
                if (req) {
                    ssh_queue_push(&app->ssh_context, req);
                }
                
                SDL_free(path_to_list);
            }
        }
    }
}

static void handle_treenode_click(
    Clay_ElementId element_id,
    Clay_PointerData pointer_data,
    intptr_t user_data
);

static void RenderFileTreeRecursive(app_t* app, RemoteFileNode* node, int indent_level) {
    //printf("%s: %d\n", __func__, indent_level);
    Clay_TextElementConfig* text_config = CLAY_TEXT_CONFIG({
        .fontId = FONT_ID_BODY_16,
        .fontSize = 16,
        .textColor = COLOR_WHITE
    });
    
    Clay_TextElementConfig* icon_config = CLAY_TEXT_CONFIG({
        .fontId = FONT_ID_BODY_16,
        .fontSize = 16,
        .textColor = COLOR_ICON_BLUE
    });

    while (node != NULL) {
        CLAY_AUTO_ID({
            .layout = {
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                .sizing = { .width = CLAY_SIZING_GROW(0) },
                .padding = { .left = indent_level * 16.0f },
                .childGap = 4,
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }
            }
        }) {
            if (node->type == REMOTE_FILE_TYPE_DIRECTORY) {
                const char* icon = node->is_expanded ? "- " : "+ ";
                CLAY_AUTO_ID({
                    .layout = { .padding = {4, 4, 0, 0}}
                }) {
                    Clay_OnHover(handle_treenode_click, (intptr_t)node);
                    CLAY_TEXT(((Clay_String){ .chars = (char*)icon, .length = strlen(icon) }), icon_config);
                }
            } else {
                CLAY_AUTO_ID({
                    .layout = { .sizing = { .width = CLAY_SIZING_FIXED(icon_config->fontSize + 8)}}
                }) {
                    CLAY_TEXT(CLAY_STRING("  "), text_config);
                }
            }
            CLAY_AUTO_ID({
                .layout = { .sizing = { .width = CLAY_SIZING_FIXED(icon_config->fontSize + 8)}}
            }) {
                CLAY_TEXT(((Clay_String){ .chars = node->name, .length = strlen(node->name) }), text_config);
            }
        }
        if (node->is_expanded && node->children) {
            RenderFileTreeRecursive(app, node->children, indent_level + 1);
        }
        node = node->next_sibling;
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
                
                if (fileMenuVisible) {
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
        CLAY(CLAY_ID("Main"), {
            .layout = { 
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                .sizing = layoutExpand, 
                .childGap = 2 
            }
        }) {
            CLAY(CLAY_ID("LeftPane"), {
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
                CLAY(CLAY_ID("ConnectStringInput"), {
                    .layout = { 
                        .sizing = { .width = CLAY_SIZING_GROW(0) },
                        .padding = CLAY_PADDING_ALL(8)
                    },
                    .backgroundColor = COLOR_NEAR_BLACK,
                    .cornerRadius = CLAY_CORNER_RADIUS(4)
                }) {
                    CLAY_TEXT(CLAY_STRING("b@192.168.1.149"), CLAY_TEXT_CONFIG({
                        .fontId = FONT_ID_BODY_16,
                        .fontSize = 16,
                        .textColor = COLOR_WHITE
                    }));
                }
                CLAY(CLAY_ID("ConnectButton"), {
                    .layout = { 
                        .padding = { 8, 8, 4, 4 },
                        .sizing = { .width = CLAY_SIZING_GROW(0) }
                    },
                    .backgroundColor = Clay_Hovered() ? COLOR_MED_GREY : COLOR_DARK_GREY,
                    .cornerRadius = CLAY_CORNER_RADIUS(4)
                }) {
                    Clay_OnHover(handle_connect_button_interaction, (intptr_t)app);
                    CLAY_TEXT(CLAY_STRING("Connect"), CLAY_TEXT_CONFIG({
                        .fontId = FONT_ID_BODY_16,
                        .fontSize = 16,
                        .textColor = COLOR_WHITE
                    }));
                }

                // TODO: run ls on an ssh channel?
                CLAY(CLAY_ID("FileTreeContainer"), {
                    .layout = {
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        .sizing = { 
                            .width = CLAY_SIZING_GROW(0),
                            .height = CLAY_SIZING_GROW(0)
                        },
                        .childGap = 2,
                    },
                    .clip = { 
                        .vertical = true,
                        .childOffset = Clay_GetScrollOffset()
                    }
                }) {
                    if (app->file_tree_root) {
                        RenderFileTreeRecursive(app, app->file_tree_root, 0);
                    } else if (app->ssh_context.is_connected) {
                        CLAY_TEXT(CLAY_STRING("Loading..."), CLAY_TEXT_CONFIG({
                            .fontId = FONT_ID_BODY_16,
                            .fontSize = 16,
                            .textColor = COLOR_WHITE
                        }));
                    } else {
                         CLAY_TEXT(CLAY_STRING("Not Connected."), CLAY_TEXT_CONFIG({
                            .fontId = FONT_ID_BODY_16,
                            .fontSize = 16,
                            .textColor = COLOR_WHITE
                        }));
                    }
                }
            }
            CLAY(CLAY_ID("LeftSideBarPaneResizer"), {
                .backgroundColor = Clay_Hovered() ? COLOR_LIGHT_GREY : COLOR_DARK_GREY,
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_FIXED(8),
                        .height = CLAY_SIZING_GROW(0)
                    }
                }
            }) {
                Clay_OnHover(handle_side_pane_resizer_interaction, (intptr_t)app);
            }
            CLAY(CLAY_ID("RightPane"), {
                .backgroundColor = { 20, 20, 20, 255 },
                .layout = {
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) },
                }
            }) {
                CLAY(CLAY_ID("FileEditor"), {
                    .backgroundColor = { 20, 20, 20, 255 },
                    .layout = {
                        .sizing = { 
                            .width = CLAY_SIZING_GROW(0), 
                            .height = CLAY_SIZING_GROW(0) 
                        },
                    }
                }) {
                    CLAY_TEXT(CLAY_STRING("{todo file editor here}"), CLAY_TEXT_CONFIG({
                        .fontId = FONT_ID_BODY_16,
                        .fontSize = 16,
                        .textColor = { 255, 255, 255, 255 }
                    }));
                }

                CLAY(CLAY_ID("RightHorizontalPaneResizer"), {
                    .backgroundColor = Clay_Hovered() ? COLOR_LIGHT_GREY : COLOR_DARK_GREY,
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                            .height = CLAY_SIZING_FIXED(8)
                        }
                    }
                }) {
                    //Clay_OnHover(handle_pane_resizer_interaction, (intptr_t)app);
                }

                CLAY(CLAY_ID("Terminal"), {
                    .backgroundColor = { 20, 20, 20, 255 }, // Dark background for the game
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                            .height = CLAY_SIZING_FIXED(100)
                        }
                    }
                }) {
                    CLAY_TEXT(CLAY_STRING("{todo terminal here}"), CLAY_TEXT_CONFIG({
                        .fontId = FONT_ID_BODY_16,
                        .fontSize = 16,
                        .textColor = { 255, 255, 255, 255 }
                    }));
                }

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
