#include <string.h>

#include "game_object_manager.h"
#include "logging.h"

static GameObjectManager go_manager;

void GameObjectManager_Init(void)
{
    go_manager.next_object_id = 0;
    go_manager.game_object_count = 0;

    memset(go_manager.game_objects, 0, sizeof(go_manager.game_objects));
}

GameObject *GameObjectManager_CreateGameObject(void)
{
    if (go_manager.game_object_count >= MAX_GAME_OBJECTS)
    {
        LogError("Cannot create a new game object: the maximum number of game objects has been reached");

        return NULL;
    }

    // find an available game object slot

    GameObject *game_object = NULL;

    for (unsigned int i = 0; i < MAX_GAME_OBJECTS; i++)
    {
        if (!go_manager.game_objects[i].is_active)
        {
            game_object = &go_manager.game_objects[i].object;
            go_manager.game_objects[i].is_active = true;

            break;
        }
    }

    if (game_object == NULL)
    {
        LogError("Cannot create a new game object: failed to find an available slot");

        return NULL;
    }

    game_object->id = go_manager.next_object_id++;
    game_object->update = NULL;
    game_object->on_delete = NULL;

    memset(&game_object->properties, 0, sizeof(game_object->properties));

    go_manager.game_object_count++;

    return game_object;
}

bool GameObjectManager_DeleteGameObject(GameObject *game_object)
{
    for (unsigned int i = 0; i < MAX_GAME_OBJECTS; i++)
    {
        GameObjectSlot *slot = &go_manager.game_objects[i];

        if (slot->is_active && slot->object.id == game_object->id)
        {
            if (game_object->on_delete)
                game_object->on_delete(game_object);

            slot->is_active = false;
            go_manager.game_object_count--;

            return true;
        }
    }

    return false;
}

bool GameObjectManager_DeleteGameObjectById(unsigned int id)
{
    GameObject *game_object = GameObjectManager_FindGameObjectById(id);

    if (!game_object)
        return false;

    return GameObjectManager_DeleteGameObject(game_object);
}

GameObject *GameObjectManager_FindGameObjectById(unsigned int id)
{
    for (unsigned int i = 0; i < MAX_GAME_OBJECTS; i++)
    {
        GameObjectSlot *slot = &go_manager.game_objects[i];

        if (slot->is_active && slot->object.id == id)
            return &slot->object;
    }

    return NULL;
}

int GameObjectManager_UpdateGameObjects(void)
{
    for (unsigned int i = 0; i < MAX_GAME_OBJECTS; i++)
    {
        GameObjectSlot *slot = &go_manager.game_objects[i];

        if (slot->is_active && slot->object.update)
        {
            GameObject *game_object = &slot->object;

            if (game_object->update(game_object) < 0)
            {
                LogError("Failed to update game object %d", game_object->id);

                return -1;
            }
        }
    }

    return 0;
}
