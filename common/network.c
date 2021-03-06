#include <string.h>

#include "network.h"
#include "logging.h"

static int SerializeNetworkState(
        NetworkState *state, NetworkObjectType type, NBN_Stream *stream);
static int SerializeTankNetworkState(TankNetworkState *state, NBN_Stream *stream);
static int SerializeProjectileNetworkState(ProjectileNetworkState *state, NBN_Stream *stream);

void GameSnapshot_Init(GameSnapshot *game_snapshot, uint32_t id)
{
    game_snapshot->id = id;
    game_snapshot->last_processed_client_tick = 0;
    game_snapshot->event_count = 0;

    memset(game_snapshot->events, 0, sizeof(game_snapshot->events));
}

int GameSnapshot_Serialize(GameSnapshot *game_snapshot, NBN_Stream *stream)
{
    SERIALIZE_UINT(game_snapshot->id, 0, UINT_MAX);
    SERIALIZE_UINT(game_snapshot->last_processed_client_tick, 0, UINT_MAX);
    SERIALIZE_UINT(game_snapshot->event_count, 0, MAX_NETWORK_EVENTS)

    for (unsigned int i = 0; i < game_snapshot->event_count; i++)
    {
        NetworkEvent *ev = &game_snapshot->events[i];

        SERIALIZE_UINT(ev->type, 0, 1);
        SERIALIZE_UINT(ev->obj_type, 0, MAX_NETWORK_OBJECT_TYPES);
        SERIALIZE_UINT(ev->network_id, 0, UINT_MAX);

        // do not serialize the state if it's a delete event
        if (ev->type == NETWORK_EV_UPDATE)
        {
            if (SerializeNetworkState(&ev->state, ev->obj_type, stream) < 0)
                return -1;
        }
    }

    return 0;
}

static int SerializeNetworkState(NetworkState *state, NetworkObjectType type, NBN_Stream *stream)
{
    switch (type)
    {
        case NETWORK_TANK:
            return SerializeTankNetworkState(&state->tank, stream);

        case NETWORK_PROJECTILE:
            return SerializeProjectileNetworkState(&state->projectile, stream);
    }

    return 0;
}

static int SerializeTankNetworkState(TankNetworkState *state, NBN_Stream *stream)
{
    SERIALIZE_FLOAT(state->position.x, 0, MAX_POSITION_X, 2);
    SERIALIZE_FLOAT(state->position.y, 0, MAX_POSITION_Y, 2);
    SERIALIZE_UINT(state->rotation, 0, 360);
    SERIALIZE_UINT(state->turret_rotation, 0, 360);

    return 0;
}

static int SerializeProjectileNetworkState(ProjectileNetworkState *state, NBN_Stream *stream)
{
    SERIALIZE_FLOAT(state->spawn_position.x, 0, MAX_POSITION_X, 2);
    SERIALIZE_FLOAT(state->spawn_position.y, 0, MAX_POSITION_Y, 2);

    SERIALIZE_UINT(state->spawn_tick, 0, UINT_MAX);
    SERIALIZE_UINT(state->rotation, 0, 360);

    return 0;
}
