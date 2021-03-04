#include "../common/network.h"
#include "../common/projectile.h"
#include "../common/game_object_manager.h"
#include "game_server.h"
#include "projectile.h"

int Projectile_Update(GameObject *game_object)
{
    Projectile *projectile = &game_object->properties.projectile;

    projectile->position = Vector2Add(projectile->position, Vector2Scale(projectile->direction, projectile->speed));

    if (projectile->position.x >= MAX_POSITION_X ||
        projectile->position.y >= MAX_POSITION_Y ||
        projectile->position.x <= 0 ||
        projectile->position.y <= 0)
    {
        GameObjectManager_DeleteGameObject(game_object);
    }

    return 0;
}

void Projectile_OnDelete(GameObject *game_object)
{
    GameServer_DeleteNetworkObject(game_object->network_id);
}