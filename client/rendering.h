#pragma once

#include <stdbool.h>
#include <raylib.h>

#define MAX_RENDERERS 1024

typedef struct
{
    bool is_active;
    Vector2 position;
    int rotation;
} RenderingState;

typedef void (*RenderFunc)(Vector2, int, void *);
typedef void (*BuildRenderingStateFunc)(RenderingState *, void *);

typedef struct
{
    unsigned int id;
    bool is_active;
    bool is_visible;
    void *data;
    RenderFunc render_func;
    BuildRenderingStateFunc build_state_func;
    RenderingState current_state;
    RenderingState previous_state;
} Renderer;

typedef struct
{
    Texture2D atlas;
    Rectangle region;
} Sprite;

void InitRendering(void);
void RenderAll(double alpha);
Renderer *CreateRenderer(RenderFunc render_func, BuildRenderingStateFunc build_state_func, void *data);
