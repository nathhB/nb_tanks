#pragma once

#include "../common/projectile.h"
#include "rendering.h"

typedef struct
{
    Projectile projectile;
    Sprite sprite;
} ClientProjectile;

int ClientProjectile_Init(ClientProjectile *cli_projectile);
