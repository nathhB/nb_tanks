#include "../common/game_object_manager.h"
#include "../common/util.h"
#include "../common/assertion.h"
#include "network.h"
#include "tank.h"
#include "projectile.h"

// tank
static GameObject *CreateRemoteTankGameObject(NetworkState *state);
static void OnRemoteTankUpdate(GameObject *tank_object, NetworkState *state);
static void InterpolateRemoteTank(
        GameObject *tank_object, unsigned int tick, InterpolationState *state1, InterpolationState *state2);

// projectile
static GameObject *CreateRemoteProjectileGameObject(NetworkState *state);
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

static GameObject *CreateRemoteTankGameObject(NetworkState *state)
{
    GameObject *game_object = GameObjectManager_CreateGameObject();

    if (ClientTank_Init(&game_object->properties.cli_tank) < 0)
        return NULL;

    OnRemoteTankUpdate(game_object, state);

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

static GameObject *CreateRemoteProjectileGameObject(NetworkState *state)
{
    GameObject *game_object = GameObjectManager_CreateGameObject();
    ClientProjectile *cli_projectile = &game_object->properties.cli_projectile;

    if (ClientProjectile_Init(cli_projectile) < 0)
        return NULL;

    cli_projectile->projectile.position = state->projectile.initial_position;

    return game_object;
}

static void OnRemoteProjectileUpdate(GameObject *projectile_object, NetworkState *state)
{
}
