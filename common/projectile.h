#pragma once

#include "raylib.h"

typedef struct
{
    Vector2 position;
    Vector2 spawn_position; // position at which the projectile has been spawned
    Vector2 direction;
    Vector2 size;
    unsigned int rotation;
    unsigned int speed;
    unsigned int shooter_client_id;
    unsigned int spawn_tick; // tick at which the projectile has been spawned
} Projectile;

void Projectile_Init(Projectile *projectile);
