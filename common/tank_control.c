#include "game_object.h"
#include "input.h"
#include "tank.h"
#include "tank_control.h"
#include "projectile_update.h"
#include "util.h"
#include "game_loop.h"
#include "logging.h"

#ifdef NB_TANKS_SERVER

#include "../server/game_server.h"

static void SpawnServerProjectile(
        Tank *tank, unsigned int shooter_client_id, unsigned int server_tick, unsigned int client_tick);

#endif

#ifdef NB_TANKS_CLIENT

#include "../client/game_client.h"
#include "projectile_update.h"
#include "game_object_manager.h"

static int SpawnClientProjectile(Tank *tank);

#endif

static void MoveForward(GameObject *game_object);
static void MoveBackward(GameObject *game_object);
static void RotateRight(GameObject *game_object);
static void RotateLeft(GameObject *game_object);
static void RotateTurretRight(GameObject *game_object);
static void RotateTurretLeft(GameObject *game_object);
static void Rotate(Tank *tank, int amount);
static void RotateTurret(Tank *tank, int amount);
static int Shoot(GameObject *game_object, Input *input, unsigned int tick);
static void InitProjectile(Projectile *projectile, Tank *tank);
static Tank *GetTank(GameObject *game_object);

int Tank_Update(GameObject *game_object, unsigned int tick)
{
#ifdef NB_TANKS_SERVER
    Client *client = GameServer_FindClientById(game_object->properties.serv_tank.client_id);
    Input *input = &client->current_input;
#endif

#ifdef NB_TANKS_CLIENT
    Input *input = GameClient_GetCurrentInput();
#endif

    if (Tank_ProcessInputs(game_object, input, tick) < 0)
    {
        LogError("Failed to process tank inputs");

        return -1;
    }

#ifdef NB_TANKS_SERVER
    client->last_processed_client_tick = input->client_tick;
#endif

    return 0;
}

int Tank_ProcessInputs(GameObject *game_object, Input *input, unsigned int tick)
{
    if ((input->keys & INPUT_UP) == INPUT_UP)
    {
        MoveForward(game_object);
    }
    else if ((input->keys & INPUT_DOWN) == INPUT_DOWN)
    {
        MoveBackward(game_object);
    }
    else if ((input->keys & INPUT_RIGHT) == INPUT_RIGHT)
    {
        RotateRight(game_object);
    }
    else if ((input->keys & INPUT_LEFT) == INPUT_LEFT)
    {
        RotateLeft(game_object);
    }

    if ((input->keys & INPUT_RIGHT_2) == INPUT_RIGHT_2)
    {
        RotateTurretRight(game_object);
    }
    else if ((input->keys & INPUT_LEFT_2) == INPUT_LEFT_2)
    {
        RotateTurretLeft(game_object);
    }

    if ((input->keys & INPUT_SPACE) == INPUT_SPACE)
    {
        if (Shoot(game_object, input, tick) < 0)
            return -1;
    }

    return 0;
}

static void MoveForward(GameObject *game_object)
{
    Tank *tank = GetTank(game_object);

    tank->position = Vector2Add(tank->position, Vector2Scale(tank->direction, tank->move_speed));
}

static void MoveBackward(GameObject *game_object)
{
    Tank *tank = GetTank(game_object);

    tank->position = Vector2Add(tank->position, Vector2Scale(tank->direction, -tank->move_speed));
}

static void RotateRight(GameObject *game_object)
{
    Tank *tank = GetTank(game_object);

    Rotate(tank, tank->rotation_speed);
}

static void RotateLeft(GameObject *game_object)
{
    Tank *tank = GetTank(game_object);

    Rotate(tank, -tank->rotation_speed);
}

static void RotateTurretRight(GameObject *game_object)
{
    Tank *tank = GetTank(game_object);

    RotateTurret(tank, tank->rotation_speed);
}

static void RotateTurretLeft(GameObject *game_object)
{
    Tank *tank = GetTank(game_object);

    RotateTurret(tank, -tank->rotation_speed);
}

static void Rotate(Tank *tank, int amount)
{
    tank->rotation = ((tank->rotation + amount) + 360) % 360; 
    tank->direction = AngleToDirection(tank->rotation);
}

static void RotateTurret(Tank *tank, int amount)
{
    tank->turret_rotation = ((tank->turret_rotation + amount) + 360) % 360; 
    tank->turret_direction = AngleToDirection(tank->turret_rotation);
}

static int Shoot(GameObject *game_object, Input *input, unsigned tick)
{
    Tank *tank = GetTank(game_object);

    if (tick - tank->last_shoot_tick >= (tank->shoot_rate * TICKS_PER_SECOND))
    {
#ifdef NB_TANKS_SERVER 
        unsigned int shooter_client_id = game_object->properties.serv_tank.client_id;

        SpawnServerProjectile(tank, shooter_client_id, tick, input->client_tick);
#endif

#ifdef NB_TANKS_CLIENT
        if (game_object->properties.cli_tank.is_local)
        {
            if (SpawnClientProjectile(tank) < 0)
                return -1;
        }
#endif

        tank->last_shoot_tick = tick;
    }

    return 0;
}

#ifdef NB_TANKS_SERVER

static void SpawnServerProjectile(
        Tank *tank, unsigned int shooter_client_id, unsigned int server_tick, unsigned int client_tick)
{
    NetworkObject *net_object = GameServer_CreateNetworkObject(NETWORK_PROJECTILE);
    GameObject *projectile_object = net_object->game_object;
    Projectile *projectile = &projectile_object->properties.projectile;
    
    InitProjectile(projectile, tank);

    projectile->spawn_position = projectile->position;
    projectile->spawn_tick = client_tick;
    projectile->shooter_client_id = shooter_client_id; 

    // compute the "one way" latency in term of tick offset between the client and server
    // shoot times
    
    unsigned tick_offset = server_tick - client_tick;

    // use the tick offset to advance the projectile further to sync its position between the client
    // and the server
    
    projectile->position = Projectile_ComputePosition(projectile, tick_offset); 

    LogDebug("Spawned projectile %d. Direction: (%f, %f). Tick offset: %d",
            net_object->id, projectile->direction.x, projectile->direction.y, tick_offset);
}

#endif

#ifdef NB_TANKS_CLIENT

static int SpawnClientProjectile(Tank *tank)
{
    GameObject *projectile_object = GameObjectManager_CreateGameObject();
    ClientProjectile *cli_projectile = &projectile_object->properties.cli_projectile;

    projectile_object->update = Projectile_Update;
    projectile_object->on_delete = Projectile_OnDelete;

    if (ClientProjectile_Init(cli_projectile, true) < 0)
        return -1;

    InitProjectile(&cli_projectile->projectile, tank);

    LogDebug("Spawned local projectile. Direction: (%f, %f)",
            cli_projectile->projectile.direction.x, cli_projectile->projectile.direction.y);

    return 0;
}

#endif

static void InitProjectile(Projectile *projectile, Tank *tank)
{
    Vector2 pos_offset = Vector2Scale(tank->turret_direction, 20);

    projectile->position = Vector2Add(tank->position, pos_offset);
    projectile->rotation = tank->turret_rotation;
    projectile->direction = AngleToDirection(tank->turret_rotation);
}

static Tank *GetTank(GameObject *game_object)
{
#ifdef NB_TANKS_SERVER
    return &game_object->properties.serv_tank.tank;
#endif

#ifdef NB_TANKS_CLIENT
    return &game_object->properties.cli_tank.tank;
#endif
}
