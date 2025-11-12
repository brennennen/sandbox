#ifndef BTERM_UI_H
#define BTERM_UI_H

#include "clay.h"
#include <stdlib.h>
#include "app.h"

ui_data_t ui_data_initialize();
void set_ui_global_context(app_t* app);
app_t* get_ui_global_context();
Clay_RenderCommandArray ui_create_layout(app_t *app);

Clay_Dimensions SDL_MeasureText(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData);


void Clay_TextInput(
    app_t* app, 
    Clay_ElementId id, 
    Clay_String* string_storage, 
    size_t buffer_size, 
    Clay_TextElementConfig* config
);



#endif // BTERM_UI_H
