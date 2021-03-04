#pragma once

#include "raylib.h"
#include "input.h"

typedef struct
{
    Vector2 position;
    Vector2 direction;
    Vector2 turret_direction;
    Vector2 size;
    float move_speed;
    unsigned int rotation_speed;
    unsigned int turret_rotation_speed;
    int rotation;
    int turret_rotation;
    unsigned int last_shoot_tick;
    unsigned int shoot_rate; // projectiles per second
} Tank;

void Tank_Init(Tank *tank);
void Tank_ProcessInputs(Tank *tank, Input *input, unsigned int tick);
void Tank_UpdateDirection(Tank *tank);
void Tank_UpdateTurretDirection(Tank *tank);
