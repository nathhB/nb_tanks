#pragma once

#include "../common/network.h"
#include "../common/game_object.h"

#define SEND_GAME_SNAPSHOTS_FREQUENCY 10 // per seconds

typedef GameObject *(*CreateGameObjectFunc)(void);
typedef bool (*IsSnapshotUpdateNeededFunc)(NetworkState *, NetworkState *);
typedef void (*UpdateNetworkStateFunc)(GameObject *, NetworkState *);

typedef struct
{
    CreateGameObjectFunc create_game_object;
    IsSnapshotUpdateNeededFunc is_snapshot_update_needed;
    UpdateNetworkStateFunc update_network_state;
} NetworkObjectBlueprint;

typedef struct
{
    uint32_t id;
    NetworkObjectType type;
    GameObject *game_object;
    NetworkState state;
    IsSnapshotUpdateNeededFunc is_snapshot_update_needed;
    UpdateNetworkStateFunc update_network_state;
} NetworkObject;

int GameSnapshot_AddEvent(
        GameSnapshot *game_snapshot, NetworkEventType type, NetworkObject *network_object);
NetworkEvent *GameSnapshot_FindNetworkEventById(
        GameSnapshot *game_snapshot, unsigned int network_id);

bool GetNetworkObjectBlueprint(NetworkObjectType type, NetworkObjectBlueprint *blueprint);
