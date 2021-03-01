#pragma once

#include "tank.h"
#include "projectile.h"

#ifdef NB_TANKS_CLIENT
#include "../client/tank.h"
#include "../client/projectile.h"
#endif

typedef struct __GameObject GameObject;

typedef int (*UpdateFunc)(GameObject *);

struct __GameObject
{
    unsigned int id;
    UpdateFunc update;

    union
    {
#ifdef NB_TANKS_CLIENT
        ClientTank cli_tank;
        ClientProjectile cli_projectile;
#endif

#ifdef NB_TANKS_SERVER
        Tank tank;
        Projectile projectile;
#endif
    } properties;
};
