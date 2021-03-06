#include "network.h"
#include "projectile.h"
#include "game_object_manager.h"
#include "projectile_update.h"

#ifdef NB_TANKS_SERVER
#include "../server/game_server.h"
#endif

int Projectile_Update(GameObject *game_object, unsigned int tick)
{
#ifdef NB_TANKS_SERVER
    Projectile *projectile = &game_object->properties.projectile;
#endif

#ifdef NB_TANKS_CLIENT
    ClientProjectile *cli_projectile = &game_object->properties.cli_projectile;
    Projectile *projectile = &cli_projectile->projectile;

    // wait for the projectie to be synced with the remote tank interpolation tick
    if (tick < cli_projectile->interp_sync_tick)
        return 0;

    // create the renderer if it's not already created
    if (!cli_projectile->renderer)
    {
        if (ClientProjectile_CreateRenderers(cli_projectile) < 0)
        {
            LogError("Failed to create projectile renderer");

            return -1;
        }
    }

    if (cli_projectile->is_catching_up)
    {
        if (tick >= cli_projectile->catchup_tick)
        {
            cli_projectile->projectile.speed = 5; // reset the speed
            cli_projectile->is_catching_up = false;
        }
    }
#endif

    projectile->position = Projectile_ComputePosition(projectile, 1);

    if (projectile->position.x >= MAX_POSITION_X ||
        projectile->position.y >= MAX_POSITION_Y ||
        projectile->position.x <= 0 ||
        projectile->position.y <= 0)
    {
        GameObjectManager_DeleteGameObject(game_object);
    }

    return 0;
}

Vector2 Projectile_ComputePosition(Projectile *projectile, unsigned int ticks)
{
    return Vector2Add(
            projectile->position,
            Vector2Scale(Vector2Scale(projectile->direction, projectile->speed), ticks));
}

void Projectile_OnDelete(GameObject *game_object)
{
#ifdef NB_TANKS_SERVER
    GameServer_DeleteNetworkObject(game_object->network_id);
#endif

#ifdef NB_TANKS_CLIENT
    // TODO: delete renderers
#endif
}
