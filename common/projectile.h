#pragma once

#include "raylib.h"

typedef struct
{
    Vector2 position;
    Vector2 direction;
    Vector2 size;
    unsigned int rotation;
    unsigned int speed;
} Projectile;

void Projectile_Init(Projectile *projectile);
