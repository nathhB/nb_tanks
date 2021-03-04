#include "projectile.h"
#include "../common/game_object.h"
#include "../common/game_object_manager.h"
#include "../common/network.h"
#include "../common/logging.h"

void Projectile_Init(Projectile *projectile)
{
    projectile->position = Vector2Zero();
    projectile->direction = Vector2Zero();
    projectile->size = (Vector2){ 8, 12 };
    projectile->rotation = 0;
    projectile->speed = 10;
}