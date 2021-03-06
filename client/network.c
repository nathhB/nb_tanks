#include "../common/game_object_manager.h"
#include "../common/util.h"
#include "../common/assertion.h"
#include "../common/tank_control.h"
#include "../common/projectile_update.h"
#include "game_client.h"
#include "network.h"
#include "tank.h"

// tank
static GameObject *CreateRemoteTankGameObject(NetworkObjectProxy *remote_object);
static void OnRemoteTankUpdate(GameObject *tank_object, NetworkState *state);
static void InterpolateRemoteTank(
        GameObject *tank_object, unsigned int tick, InterpolationState *state1, InterpolationState *state2);

// projectile
static GameObject *CreateRemoteProjectileGameObject(NetworkObjectProxy *remote_object);
static void OnRemoteProjectileUpdate(GameObject *tank_object, NetworkState *state);

bool GetNetworkObjectProxyBlueprint(NetworkObjectType type, NetworkObjectProxyBlueprint *blueprint)
{
    if (type == NETWORK_TANK)
    {
        blueprint->create_game_object = CreateRemoteTankGameObject;
        blueprint->on_state_update = OnRemoteTankUpdate;
        blueprint->lag_compensation_policy = LAG_COMPENSATION_INTERP;
        blueprint->lag_compensation.interpolation_func = InterpolateRemoteTank;

        return true;
    }
    else if (type == NETWORK_PROJECTILE)
    {
        blueprint->create_game_object = CreateRemoteProjectileGameObject;
        blueprint->on_state_update = OnRemoteProjectileUpdate;
        blueprint->lag_compensation_policy = LAG_COMPENSATION_NONE;

        return true;
    }

    return false;
}

static GameObject *CreateRemoteTankGameObject(NetworkObjectProxy *remote_object)
{
    GameObject *game_object = GameObjectManager_CreateGameObject();

    game_object->update = Tank_Update;

    bool is_local = remote_object->id == GameClient_GetLocalTankNetworkId();

    if (ClientTank_Init(&game_object->properties.cli_tank, is_local) < 0)
        return NULL;

    OnRemoteTankUpdate(game_object, &remote_object->state);

    return game_object;
}

static void OnRemoteTankUpdate(GameObject *tank_object, NetworkState *state)
{
    Tank *tank = &tank_object->properties.cli_tank.tank;

    tank->position = state->tank.position;
    tank->rotation = state->tank.rotation;
    tank->turret_rotation = state->tank.turret_rotation;
}

static void InterpolateRemoteTank(
        GameObject *tank_object, unsigned int tick, InterpolationState *interp_state1, InterpolationState *interp_state2)
{
    unsigned int length = interp_state2->tick - interp_state1->tick;
    unsigned int t = tick - interp_state1->tick;
    float alpha = (length > 0) ? (float)t / length : 0;

    Tank *tank = &tank_object->properties.cli_tank.tank;

    tank->position = Vector2Lerp(interp_state1->state.tank.position, interp_state2->state.tank.position, alpha);
    tank->rotation = LerpRotation(interp_state1->state.tank.rotation, interp_state2->state.tank.rotation, alpha);
    tank->turret_rotation = LerpRotation(
            interp_state1->state.tank.turret_rotation, interp_state2->state.tank.turret_rotation, alpha);
}

static GameObject *CreateRemoteProjectileGameObject(NetworkObjectProxy *remote_object)
{
    GameObject *game_object = GameObjectManager_CreateGameObject();
    ClientProjectile *cli_projectile = &game_object->properties.cli_projectile;

    game_object->update = Projectile_Update;
    game_object->on_delete = Projectile_OnDelete;

    if (ClientProjectile_Init(cli_projectile, false) < 0)
        return NULL;

    // delay the remote projectile by the number of interpolated ticks
    cli_projectile->interp_sync_tick = GameClient_GetCurrentTick() + INTERPOLATION_TICKS - 2;

    cli_projectile->projectile.position = remote_object->state.projectile.spawn_position;
    cli_projectile->projectile.rotation = remote_object->state.projectile.rotation;
    cli_projectile->projectile.direction = AngleToDirection(remote_object->state.projectile.rotation); 

    // compute catchup information to synchronize the remote projectile with the server
    unsigned int tick_offset = GameClient_GetCurrentTick() - remote_object->state.projectile.spawn_tick;
    Vector2 catchup_position = Projectile_ComputePosition(
            &cli_projectile->projectile, tick_offset * 2 + INTERPOLATION_TICKS);

    // compute the "catchup" speed of the remote projectile
    float catchup_speed = Vector2Distance(cli_projectile->projectile.position, catchup_position) / tick_offset;

    cli_projectile->projectile.speed = catchup_speed;
    cli_projectile->catchup_tick = GameClient_GetCurrentTick() + tick_offset;

    LogDebug("Created remote projectile %d. Position: (%f, %f). Direction: (%f %f). Tick offset: %d. Catchup speed: %f",
            remote_object->id,
            cli_projectile->projectile.position.x, cli_projectile->projectile.position.y,
            cli_projectile->projectile.direction.x, cli_projectile->projectile.direction.y,
            tick_offset,
            catchup_speed);

    return game_object;
}

static void OnRemoteProjectileUpdate(GameObject *projectile_object, NetworkState *state)
{
}
