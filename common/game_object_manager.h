#pragma once

#include <stdbool.h>

#include "game_object.h"

#define MAX_GAME_OBJECTS 1024

typedef struct
{
    GameObject object;
    bool is_active;
} GameObjectSlot;

typedef struct
{
    unsigned int next_object_id;
    GameObjectSlot game_objects[MAX_GAME_OBJECTS];
    unsigned int game_object_count;
} GameObjectManager;

void GameObjectManager_Init(void);
GameObject *GameObjectManager_CreateGameObject(void);
bool GameObjectManager_DeleteGameObject(GameObject *game_object);
void GameObjectManager_UpdateGameObjects(void);
