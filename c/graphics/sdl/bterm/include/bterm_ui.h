#ifndef BTERM_UI_H
#define BTERM_UI_H

#include "clay.h"
#include <stdlib.h>
#include "app.h"

ui_data_t ui_data_initialize();
Clay_RenderCommandArray ui_create_layout(app_t *app);

#endif // BTERM_UI_H
