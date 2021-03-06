#pragma once

#include "raylib.h"

typedef struct
{
    Vector2 position;
    Vector2 direction;
    Vector2 turret_direction;
    Vector2 size;
    float move_speed;
    float rotation_speed;
    float turret_rotation_speed;
    int rotation;
    int turret_rotation;
    unsigned int last_shoot_tick;
    unsigned int shoot_rate; // projectiles per second
} Tank;

void Tank_Init(Tank *tank);
