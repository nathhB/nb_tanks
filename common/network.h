#pragma once

#include <limits.h>

#include "raylib.h"
#include "nbnet.h"
#include "input.h"

#define PROTOCOL_NAME "nb_tanks-v1"
#define PORT 42042
#define SERVER_FULL_CODE 1
#define MAX_CLIENTS 8
#define MAX_NETWORK_OBJECTS 512
#define MAX_NETWORK_EVENTS (MAX_NETWORK_OBJECTS*2)
#define MAX_NETWORK_OBJECT_TYPES 8
#define MAX_POSITION_X 800
#define MAX_POSITION_Y 600

typedef enum
{
    NETWORK_TANK,
    NETWORK_PROJECTILE
} NetworkObjectType;

typedef enum
{
    NETWORK_EV_UPDATE,
    NETWORK_EV_DELETE
} NetworkEventType;

typedef struct
{
    Vector2 position;
    int rotation;
    int turret_rotation;
} TankNetworkState;

typedef struct
{
    Vector2 initial_position;
    int rotation;
} ProjectileNetworkState;

typedef union
{
    TankNetworkState tank;
    ProjectileNetworkState projectile;
} NetworkState;

typedef struct
{
    NetworkEventType type;
    NetworkObjectType obj_type;
    uint32_t network_id;
    NetworkState state;
} NetworkEvent;

enum
{
    CLIENT_GAME_CLOCK_SYNC_MESSAGE,
    SERVER_GAME_CLOCK_SYNC_MESSAGE,
    INPUT_MESSAGE,
    GAME_SNAPSHOT_MESSAGE,
    ACK_GAME_SNAPSHOT_MESSAGE
};

// Game clock synchronization messages

typedef struct
{
    unsigned int tick;
} ClientGameClockSyncMessage;

typedef struct
{
    unsigned int client_tick;
    unsigned int server_tick;
} ServerGameClockSyncMessage;

BEGIN_MESSAGE(ClientGameClockSyncMessage)
    SERIALIZE_UINT(msg->tick, 0, UINT_MAX);
END_MESSAGE

BEGIN_MESSAGE(ServerGameClockSyncMessage)
    SERIALIZE_UINT(msg->client_tick, 0, UINT_MAX);
    SERIALIZE_UINT(msg->server_tick, 0, UINT_MAX);
END_MESSAGE

// ------------------------------

typedef struct
{
    Input input;
} InputMessage;

typedef struct
{
    uint32_t id;
    uint32_t last_processed_input_id;
    unsigned int event_count;
    NetworkEvent events[MAX_NETWORK_EVENTS];
} GameSnapshot;

typedef struct
{
    GameSnapshot game_snapshot;
} GameSnapshotMessage;

typedef struct
{
    uint32_t id; // tick of the acked game snapshot
} AckGameSnapshotMessage;

void GameSnapshot_Init(GameSnapshot *game_snapshot, uint32_t id);
int GameSnapshot_Serialize(GameSnapshot *game_snapshot, NBN_Stream *stream);

// int SerializeNetworkObject(NetworkObject *object, NBN_Stream *stream);

BEGIN_MESSAGE(InputMessage)
    SERIALIZE_UINT(msg->input.id, 0, UINT_MAX);
    SERIALIZE_UINT(msg->input.keys, 0, UINT_MAX);
END_MESSAGE

BEGIN_MESSAGE(GameSnapshotMessage)
    GameSnapshot_Serialize(&msg->game_snapshot, stream);
END_MESSAGE

BEGIN_MESSAGE(AckGameSnapshotMessage)
    SERIALIZE_UINT(msg->id, 0, UINT_MAX);
END_MESSAGE
