#include "../common/game_object_manager.h"
#include "../common/projectile.h"
#include "network.h"

// tank
static bool IsTankSnapshotUpdateNeeded(NetworkState *state, NetworkState *last_acked_state);
static GameObject *CreateTankGameObject(void);
static void UpdateTankNetworkState(GameObject *game_object, NetworkState *state);

// projectile
static bool IsProjectileSnapshotUpdateNeeded(NetworkState *state, NetworkState *last_acked_state);
static GameObject *CreateProjectileGameObject(void);
static void UpdateProjectileNetworkState(GameObject *game_object, NetworkState *state);

int GameSnapshot_AddEvent(
        GameSnapshot *game_snapshot, NetworkEventType type, NetworkObject *network_object)
{
    if (game_snapshot->event_count >= MAX_NETWORK_EVENTS)
    {
        LogError("Failed to add network object to game snapshot: snapshot is full");

        return -1;
    }

    NetworkEvent *ev = &game_snapshot->events[game_snapshot->event_count];

    ev->type = type;
    ev->obj_type = network_object->type;
    ev->network_id = network_object->id;
    
    if (ev->type == NETWORK_EV_UPDATE)
        memcpy(&ev->state, &network_object->state, sizeof(NetworkState));

    game_snapshot->event_count++;

    return 0;
}

NetworkEvent *GameSnapshot_FindNetworkEventById(
        GameSnapshot *game_snapshot, unsigned int network_id)
{
    for (unsigned int i = 0; i < game_snapshot->event_count; i++)
    {
        if (game_snapshot->events[i].network_id == network_id)
            return &game_snapshot->events[i];
    }

    return NULL;
}

bool GetNetworkObjectBlueprint(NetworkObjectType type, NetworkObjectBlueprint *blueprint)
{
    if (type == NETWORK_TANK)
    {
        blueprint->create_game_object = CreateTankGameObject;
        blueprint->is_snapshot_update_needed = IsTankSnapshotUpdateNeeded;
        blueprint->update_network_state = UpdateTankNetworkState;

        return true;
    }
    else if (type == NETWORK_PROJECTILE)
    {
        blueprint->create_game_object = CreateProjectileGameObject;
        blueprint->is_snapshot_update_needed = IsProjectileSnapshotUpdateNeeded;
        blueprint->update_network_state = UpdateProjectileNetworkState;

        return true;
    }

    return false;
}

static GameObject *CreateTankGameObject(void)
{
    GameObject *game_object = GameObjectManager_CreateGameObject();

    Tank_Init(&game_object->properties.tank);

    return game_object;
}

static bool IsTankSnapshotUpdateNeeded(NetworkState *state, NetworkState *last_acked_state)
{
    return true;
}

static void UpdateTankNetworkState(GameObject *game_object, NetworkState *state)
{
    state->tank.position = game_object->properties.tank.position;
    state->tank.rotation = game_object->properties.tank.rotation;
    state->tank.turret_rotation = game_object->properties.tank.turret_rotation;
}

static bool IsProjectileSnapshotUpdateNeeded(NetworkState *state, NetworkState *last_acked_state)
{
    return true;
}

static GameObject *CreateProjectileGameObject(void)
{
    GameObject *game_object = GameObjectManager_CreateGameObject();

    Projectile_Init(&game_object->properties.projectile);

    return game_object;
}

static void UpdateProjectileNetworkState(GameObject *game_object, NetworkState *state)
{
    state->projectile.position = game_object->properties.projectile.position;
    state->projectile.rotation = game_object->properties.projectile.rotation;
}
