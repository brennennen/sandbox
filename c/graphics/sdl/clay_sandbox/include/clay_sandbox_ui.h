#ifndef CLAY_SANDBOX_UI_H
#define CLAY_SANDBOX_UI_H

#include "clay.h"
#include <stdlib.h>
#include "app.h"

clay_sandbox_ui_data_t sandbox_data_initialize();
Clay_RenderCommandArray sandbox_create_layout(app_state_t *app_state);

#endif // CLAY_SANDBOX_UI_H
