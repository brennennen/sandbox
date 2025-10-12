#ifndef CLAY_SANDBOX_UI_H
#define CLAY_SANDBOX_UI_H

#include "./clay.h"
#include <stdlib.h>

typedef struct {
    Clay_String title;
    Clay_String contents;
} document_t;

typedef struct {
    document_t *documents;
    uint32_t length;
} document_array_t;

typedef struct {
    intptr_t offset;
    intptr_t memory;
} clay_sandbox_arena_t;

typedef struct {
    int32_t selectedDocumentIndex;
    float yOffset;
    clay_sandbox_arena_t frameArena;
} clay_sandbox_data_t;

clay_sandbox_data_t sandbox_data_initialize();
Clay_RenderCommandArray sandbox_create_layout(clay_sandbox_data_t *data);

#endif // CLAY_SANDBOX_UI_H
