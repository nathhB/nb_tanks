#pragma once

#include "../common/projectile.h"
#include "rendering.h"

typedef struct
{
    bool is_local;

    // used to synchronize remote client projectiles
    // with interpolated remote tank positions
    unsigned int interp_sync_tick;

    // tick until the remote projectile has to sped up before it's synchronized
    // with the server position
    unsigned int catchup_tick;

    // is the remote projectile currently catching up
    bool is_catching_up;

    Projectile projectile;
    Renderer *renderer;
    Sprite sprite;
} ClientProjectile;

int ClientProjectile_Init(ClientProjectile *cli_projectile, bool is_local);
int ClientProjectile_CreateRenderers(ClientProjectile *cli_projectile);
