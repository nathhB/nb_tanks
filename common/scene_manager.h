#pragma once

#include "input.h"

typedef struct
{
    void *data;
    void (*on_enter)(void);
    void (*on_exit)(void);
    int (*on_simulate)(Input *input, double);
    int (*on_draw)(double);
} Scene;

void SceneManager_RegisterScene(
        unsigned int scene_id,
        void *data,
        void (*on_enter)(void),
        void (*on_exit)(void),
        int (*on_simulate)(Input *input, double),
        int (*on_draw)(double));

void SceneManager_ChangeScene(unsigned int scene_id);
Scene *SceneManager_GetCurrentScene(void);
