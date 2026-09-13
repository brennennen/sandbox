#ifndef DEBUG_UI_H
#define DEBUG_UI_H

#include "engine/core/game_engine.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

bool debug_imgui_init(game_engine_t* engine);
bool debug_imgui_process_event(const void* native_event);
void debug_imgui_begin_frame();
void debug_imgui_render(graphics_t* graphics);
void debug_imgui_shutdown(game_engine_t* engine);

#endif
