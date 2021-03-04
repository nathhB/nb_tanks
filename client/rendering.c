#include <string.h>

#include "rendering.h"
#include "../common/raylib.h"
#include "../common/logging.h"
#include "../common/assertion.h"
#include "../common/util.h"

static Renderer *FindAvailableRenderer(void);

unsigned int next_renderer_id = 0;
unsigned int renderer_count = 0;
Renderer renderers[MAX_RENDERERS];

void InitRendering(void)
{
    memset(renderers, 0, sizeof(renderers));
}

void RenderAll(double alpha)
{
    for (unsigned int i = 0; i < MAX_RENDERERS; i++)
    {
        Renderer *renderer = &renderers[i];
        RenderingState *previous_state = &renderer->previous_state;
        RenderingState *current_state = &renderer->current_state;

        if (renderer->is_active)
        {
            renderer->build_state_func(current_state, renderer->data);

            current_state->is_active = true;

            Vector2 position;
            int rotation;

            if (previous_state->is_active)
            {
                position = Vector2Lerp(previous_state->position, current_state->position, alpha);
                rotation = LerpRotation(previous_state->rotation, current_state->rotation, alpha);
            }
            else
            {
                position = current_state->position;
                rotation = current_state->rotation;
            }

            renderer->render_func(position, rotation, renderer->data);

            memcpy(&renderer->previous_state, current_state, sizeof(RenderingState));
        }
    }
}

int CreateRenderer(RenderFunc render_func, BuildRenderingStateFunc build_state_func, void *data)
{
    if (renderer_count >= MAX_RENDERERS)
    {
        LogError("Cannot create renderer: max number of renderers has been reached");

        return -1;
    }

    Renderer *renderer = FindAvailableRenderer();

    Assert(renderer, "Failed to find an available renderer"); // this should never happen

    renderer->id = next_renderer_id++;
    renderer->is_active = true;
    renderer->data = data;
    renderer->render_func = render_func;
    renderer->build_state_func = build_state_func;

    memset(&renderer->current_state, 0, sizeof(RenderingState));
    memset(&renderer->previous_state, 0, sizeof(RenderingState));

    return 0;
}

static Renderer *FindAvailableRenderer(void)
{
    for (unsigned int i = 0; i < MAX_RENDERERS; i++)
    {
        Renderer *renderer = &renderers[i];

        if (!renderer->is_active)
            return renderer;
    }

    return NULL;
}
