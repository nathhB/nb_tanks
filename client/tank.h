#pragma once

#include "../common/tank.h"
#include "../common/network.h"
#include "rendering.h"

typedef struct
{
    Tank tank;
    Sprite core_sprite;
    Sprite turret_sprite;
} ClientTank;

int ClientTank_Init(ClientTank *cli_tank);
