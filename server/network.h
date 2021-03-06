#pragma once

#include "../common/network.h"
#include "../common/game_object.h"

#define INPUT_BUFFER_SIZE 8
#define GAME_SNAPSHOT_BUFFER_SIZE 16
#define SEND_GAME_SNAPSHOTS_FREQUENCY 30 // per seconds

typedef struct __NetworkObject NetworkObject;

typedef GameObject *(*CreateGameObjectFunc)(void);
typedef bool (*IsSnapshotUpdateNeededFunc)(unsigned int, NetworkObject *);
typedef void (*UpdateNetworkStateFunc)(GameObject *, NetworkState *);

typedef struct
{
    CreateGameObjectFunc create_game_object;
    IsSnapshotUpdateNeededFunc is_snapshot_update_needed;
    UpdateNetworkStateFunc update_network_state;
} NetworkObjectBlueprint;

struct __NetworkObject
{
    uint32_t id;
    NetworkObjectType type;
    GameObject *game_object;
    NetworkState state;
    NetworkState last_acked_state;
    bool has_been_acked_once;
    IsSnapshotUpdateNeededFunc is_snapshot_update_needed;
    UpdateNetworkStateFunc update_network_state;
};

int GameSnapshot_AddEvent(
        GameSnapshot *game_snapshot, NetworkEventType type, NetworkObject *network_object);
NetworkEvent *GameSnapshot_FindNetworkEventById(
        GameSnapshot *game_snapshot, unsigned int network_id);

bool GetNetworkObjectBlueprint(NetworkObjectType type, NetworkObjectBlueprint *blueprint);
