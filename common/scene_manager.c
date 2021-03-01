#include <stdlib.h>

#include "scene_manager.h"
#include "logging.h"

#define SCENE_MANAGER_MAX_SCENES 8

static Scene scenes[SCENE_MANAGER_MAX_SCENES] = {
    (Scene){ NULL, NULL, NULL, NULL, NULL }
};
static unsigned int current_scene_id = 0;

void SceneManager_RegisterScene(
        unsigned int id,
        void *data,
        void (*on_enter)(void),
        void (*on_exit)(void),
        int (*on_simulate)(Input *input, double),
        int (*on_draw)(double))
{
    scenes[id] = (Scene){ data, on_enter, on_exit, on_simulate, on_draw };
}

void SceneManager_ChangeScene(unsigned int id)
{
    unsigned int old_scene_id = current_scene_id;

    current_scene_id = id;

    LogDebug("Exited scene %d", old_scene_id);
    LogDebug("Entered scene %d", current_scene_id);

    if (scenes[old_scene_id].on_exit != NULL)
        scenes[old_scene_id].on_exit();

    if (scenes[current_scene_id].on_enter)
        scenes[current_scene_id].on_enter();
}

Scene *SceneManager_GetCurrentScene(void)
{
    return &scenes[current_scene_id];
}
