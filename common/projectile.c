#include <raymath.h>

#include "projectile.h"
#include "../common/game_object.h"

void Projectile_Init(Projectile *projectile)
{
    projectile->position = Vector2Zero();
    projectile->size = (Vector2){ 8, 12 };
}

void Projectile_Update(GameObject *projectile_object)
{
}
