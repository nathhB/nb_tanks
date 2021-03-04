#pragma once

#ifdef NB_TANKS_SERVER

#include "tank.h"
#include "projectile.h"

#endif

#ifdef NB_TANKS_CLIENT

#include "../client/tank.h"
#include "../client/projectile.h"

#endif

typedef struct __GameObject GameObject;

typedef int (*GameObjectUpdateFunc)(GameObject *);
typedef void (*GameObjectOnDeleteFunc)(GameObject *);

struct __GameObject
{
    unsigned int id;
    unsigned int network_id; // id of the attached network object or network object proxy
    GameObjectUpdateFunc update;
    GameObjectOnDeleteFunc on_delete;

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

#ifdef NB_TANKS_SERVER
#endif
};
