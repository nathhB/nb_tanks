#pragma once

#include <raymath.h>

#include "game_object.h"

typedef struct
{
    Vector2 position;
    Vector2 size;
    int rotation;
} Projectile;

void Projectile_Init(Projectile *projectile);
