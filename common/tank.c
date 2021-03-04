#include <math.h>
#include <raymath.h>

#include "tank.h"
#include "logging.h"
#include "util.h"
#include "game_loop.h"

#ifdef NB_TANKS_SERVER

#include "../server/game_server.h"
#include "../server/network.h"

static void SpawnProjectile(Tank *tank);

#endif

static void MoveForward(Tank *tank);
static void MoveBackward(Tank *tank);
static void RotateRight(Tank *tank);
static void RotateLeft(Tank *tank);
static void RotateTurretRight(Tank *tank);
static void RotateTurretLeft(Tank *tank);
static void Rotate(Tank *tank, int amount);
static void RotateTurret(Tank *tank, int amount);
static void Shoot(Tank *tank, unsigned int tick);

void Tank_Init(Tank *tank)
{
    tank->position = Vector2Zero();
    tank->move_speed = 3;
    tank->rotation_speed = 3;
    tank->turret_rotation_speed = 3;
    tank->rotation = 90;
    tank->turret_rotation = 90;
    tank->last_shoot_tick = 0;
    tank->shoot_rate = 1;

    Tank_UpdateDirection(tank);
    Tank_UpdateTurretDirection(tank);
}

void Tank_ProcessInputs(Tank *tank, Input *input, unsigned int tick)
{
    if ((input->keys & INPUT_UP) == INPUT_UP)
    {
        MoveForward(tank);
    }
    else if ((input->keys & INPUT_DOWN) == INPUT_DOWN)
    {
        MoveBackward(tank);
    }
    else if ((input->keys & INPUT_RIGHT) == INPUT_RIGHT)
    {
        RotateRight(tank);
    }
    else if ((input->keys & INPUT_LEFT) == INPUT_LEFT)
    {
        RotateLeft(tank);
    }

    if ((input->keys & INPUT_RIGHT_2) == INPUT_RIGHT_2)
    {
        RotateTurretRight(tank);
    }
    else if ((input->keys & INPUT_LEFT_2) == INPUT_LEFT_2)
    {
        RotateTurretLeft(tank);
    }

    if ((input->keys & INPUT_SPACE) == INPUT_SPACE)
    {
        Shoot(tank, tick);
    }
}

void Tank_UpdateDirection(Tank *tank)
{
    tank->direction = AngleToDirection(tank->rotation);
}

void Tank_UpdateTurretDirection(Tank *tank)
{
    tank->turret_direction = AngleToDirection(tank->turret_rotation);
}

static void MoveForward(Tank *tank)
{
    tank->position = Vector2Add(tank->position, Vector2Scale(tank->direction, tank->move_speed));
}

static void MoveBackward(Tank *tank)
{
    tank->position = Vector2Add(tank->position, Vector2Scale(tank->direction, -tank->move_speed));
}

static void RotateRight(Tank *tank)
{
    Rotate(tank, tank->rotation_speed);
}

static void RotateLeft(Tank *tank)
{
    Rotate(tank, -tank->rotation_speed);
}

static void RotateTurretRight(Tank *tank)
{
    RotateTurret(tank, tank->rotation_speed);
}

static void RotateTurretLeft(Tank *tank)
{
    RotateTurret(tank, -tank->rotation_speed);
}

static void Rotate(Tank *tank, int amount)
{
    tank->rotation = ((tank->rotation + amount) + 360) % 360; 

    Tank_UpdateDirection(tank);
}

static void RotateTurret(Tank *tank, int amount)
{
    tank->turret_rotation = ((tank->turret_rotation + amount) + 360) % 360; 

    Tank_UpdateTurretDirection(tank);
}

static void Shoot(Tank *tank, unsigned tick)
{
    if (tick - tank->last_shoot_tick >= (tank->shoot_rate * TICKS_PER_SECOND))
    {
#ifdef NB_TANKS_SERVER
        SpawnProjectile(tank);
#endif

        tank->last_shoot_tick = tick;
    }
}

#ifdef NB_TANKS_SERVER

static void SpawnProjectile(Tank *tank)
{
    NetworkObject *net_object = GameServer_CreateNetworkObject(NETWORK_PROJECTILE);
    GameObject *projectile_object = net_object->game_object;
    Projectile *projectile = &projectile_object->properties.projectile;
    Vector2 pos_offset = Vector2Scale(tank->turret_direction, 30);

    // projectile_object->update = Projectile_Update;

    projectile->position = Vector2Add(tank->position, pos_offset);
    projectile->rotation = tank->turret_rotation;
    projectile->direction = AngleToDirection(tank->turret_rotation);

    LogDebug("Spawned projectile %d", net_object->id);
}

#endif
