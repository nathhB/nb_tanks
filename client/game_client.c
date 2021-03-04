#include <string.h>

#include "game_client.h"
#include "../common/network.h"

static GameClient game_client;

static int HandleNetworkEvent(NetworkEvent *network_event);
static int HandleNetworkObjectUpdate(unsigned int id, NetworkObjectType type, NetworkState *state);
static void UpdateRemoteObjectState(NetworkObjectProxy *remote_object, NetworkState *state);
static int HandleNetworkObjectDelete(unsigned int id);
static bool GetInterpolationStates(
        NetworkObjectProxy *remote_object,
        unsigned int interp_tick,
        InterpolationState *state1,
        InterpolationState *state2);

void GameClient_Init(uint32_t network_id, Vector2 position)
{
    game_client.last_sent_input_id = 0;
    game_client.remote_object_count = 0;
    game_client.game_clock.tick = 0;
    game_client.is_game_clock_synced = false;

    memset(game_client.remote_objects, 0, sizeof(game_client.remote_objects));

    game_client.remote_tank = GameClient_CreateRemoteObject(network_id, NETWORK_TANK, true);
}

void GameClient_NextTick(void)
{
    game_client.game_clock.tick++;
}

unsigned int GameClient_GetCurrentTick(void)
{
    return game_client.game_clock.tick;
}

void GameClient_SyncGameClock(unsigned int tick)
{
    game_client.game_clock.tick = tick;
    game_client.is_game_clock_synced = true;
}

bool GameClient_IsGameClockSynchronized(void)
{
    return game_client.is_game_clock_synced;
}

ClientTank *GameClient_GetClientTank(void)
{
    return &game_client.remote_tank->game_object->properties.cli_tank;
}

int GameClient_SendInputMessage(Input *input)
{
    InputMessage *msg = NBN_GameClient_CreateUnreliableMessage(INPUT_MESSAGE);

    if (!msg)
        return -1;

    memcpy(&msg->input, input, sizeof(Input));

    if (NBN_GameClient_SendMessage() < 0)
        return -1;

    game_client.last_sent_input_id = input->id;

    return 0;
}

void GameClient_BufferInput(Input *input)
{
    memcpy(&game_client.input_buffer[input->id % INPUT_BUFFER_SIZE], input, sizeof(Input));
}

unsigned int GameClient_GetPredictionInputs(
        unsigned int last_server_processed_input_id, Input inputs[INPUT_BUFFER_SIZE])
{
    unsigned int input_count = 0;

    for (
            unsigned int i = last_server_processed_input_id + 1;
            i <= game_client.last_sent_input_id && input_count < INPUT_BUFFER_SIZE;
            i++, input_count++)
    {
        inputs[input_count] = game_client.input_buffer[i % INPUT_BUFFER_SIZE];
    }

    return input_count;
}

NetworkObjectProxy *GameClient_CreateRemoteObject(unsigned int network_id, NetworkObjectType type, bool predicted)
{
    if (game_client.remote_object_count >= MAX_NETWORK_OBJECTS)
    {
        LogError("Cannot create a new remote object: the maximum number of remote objects has been reached");

        return NULL;
    }

    NetworkObjectProxyBlueprint blueprint;

    if (!GetNetworkObjectProxyBlueprint(type, &blueprint))
    {
        LogError("Cannot create a new remote object: no blueprint exist for network objects of type %d", type);

        return NULL;
    }

    // find an available remote object slot

    NetworkObjectProxy *remote_object = NULL;

    for (unsigned int i = 0; i < MAX_NETWORK_OBJECTS; i++)
    {
        if (!game_client.remote_objects[i].is_active)
        {
            remote_object = &game_client.remote_objects[i].object;
            game_client.remote_objects[i].is_active = true;

            break;
        }
    }

    if (remote_object == NULL)
    {
        LogError("Cannot create a new remote object: failed to find an available slot");

        return NULL;
    }

    memset(&remote_object->state, 0, sizeof(remote_object->state));
    memset(&remote_object->lag_compensation, 0, sizeof(remote_object->lag_compensation));

    remote_object->id = network_id;
    remote_object->type = type;
    remote_object->game_object = blueprint.create_game_object(&remote_object->state);
    remote_object->on_state_update = blueprint.on_state_update;
    remote_object->lag_compensation_policy = predicted ? LAG_COMPENSATION_NONE : blueprint.lag_compensation_policy;  

    remote_object->game_object->network_id = network_id;

    if (remote_object->lag_compensation_policy == LAG_COMPENSATION_INTERP)
    {
        remote_object->lag_compensation.interpolation.interpolation_func =
            blueprint.lag_compensation.interpolation_func;
    }

    game_client.remote_object_count++;

    return remote_object;
}

bool GameClient_DeleteRemoteObject(unsigned int network_id)
{
    for (unsigned int i = 0; i < MAX_NETWORK_OBJECTS; i++)
    {
        NetworkObjectProxySlot *slot = &game_client.remote_objects[i];

        if (slot->is_active && slot->object.id == network_id)
        {
            slot->is_active = false;
            game_client.remote_object_count--;

            return true;
        }
    }

    return false;
}

NetworkObjectProxy *GameClient_FindRemoteObject(unsigned int network_id)
{
    for (unsigned int i = 0; i < MAX_NETWORK_OBJECTS; i++)
    {
        NetworkObjectProxySlot *slot = &game_client.remote_objects[i];

        if (slot->is_active && slot->object.id == network_id)
            return &slot->object;
    }

    return NULL;
}

unsigned int GameClient_GetRemoteObjects(NetworkObjectProxy *remote_objects[MAX_NETWORK_OBJECTS])
{
    unsigned int count = 0;

    for (unsigned int i = 0; i < MAX_NETWORK_OBJECTS; i++)
    {
        if (game_client.remote_objects[i].is_active)
        {
            remote_objects[count] = &game_client.remote_objects[i].object;
            count++;
        }
    }

    return count;
}

void GameClient_InterpolateRemoteObjects(void)
{
    for (unsigned int i = 0; i < MAX_NETWORK_OBJECTS; i++)
    {
        if (game_client.remote_objects[i].is_active)
        {
            NetworkObjectProxy *remote_object = &game_client.remote_objects[i].object;

            if (remote_object->lag_compensation_policy == LAG_COMPENSATION_INTERP)
            {
                InterpolationState interp_state1;
                InterpolationState interp_state2;
                unsigned int interp_tick = GameClient_GetCurrentTick() - INTERPOLATION_TICKS;

                if (GetInterpolationStates(remote_object, interp_tick, &interp_state1, &interp_state2))
                {
                    remote_object->lag_compensation.interpolation.interpolation_func(
                            remote_object->game_object, interp_tick, &interp_state1, &interp_state2);
                }
                else
                {
                    LogWarning("Failed to run interpolation for remote object %d", remote_object->id);
                }
            }
        }
    }
}

int GameClient_AckGameSnapshot(GameSnapshot *game_snapshot)
{
    AckGameSnapshotMessage *msg = NBN_GameClient_CreateUnreliableMessage(ACK_GAME_SNAPSHOT_MESSAGE);

    if (!msg)
        return -1;

    msg->id = game_snapshot->id;

    if (NBN_GameClient_SendMessage() < 0)
        return -1;

    return 0;
}

int GameClient_ProcessGameSnapshot(GameSnapshot *game_snapshot)
{
    for (unsigned int i = 0; i < game_snapshot->event_count; i++)
    {
        NetworkEvent *network_event = &game_snapshot->events[i];

        if (HandleNetworkEvent(network_event) < 0)
        {
            LogError("Failed to process game snapshot's network events");

            return -1;
        }
    }

    return 0;
}

static int HandleNetworkEvent(NetworkEvent *network_event)
{
    if (network_event->type == NETWORK_EV_UPDATE)
    {
        if (HandleNetworkObjectUpdate(network_event->network_id, network_event->obj_type, &network_event->state) < 0)
            return -1;
    }
    else if (network_event->type == NETWORK_EV_DELETE)
    {
        if (HandleNetworkObjectDelete(network_event->network_id) < 0)
            return -1;
    }

    return 0;
}

static int HandleNetworkObjectUpdate(unsigned int id, NetworkObjectType type, NetworkState *state)
{
    NetworkObjectProxy *remote_object = GameClient_FindRemoteObject(id);

    if (!remote_object)
    {
        remote_object = GameClient_CreateRemoteObject(id, type, false);

        if (!remote_object)
            return -1;

        LogDebug("Created remote object %d of type %d", id, type);
    }

    UpdateRemoteObjectState(remote_object, state);

    return 0;
}

static void UpdateRemoteObjectState(NetworkObjectProxy *remote_object, NetworkState *state)
{
    if (remote_object->lag_compensation_policy == LAG_COMPENSATION_NONE)
    {
        memcpy(&remote_object->state, state, sizeof(NetworkState));

        remote_object->on_state_update(remote_object->game_object, &remote_object->state);
    }
    else if (remote_object->lag_compensation_policy == LAG_COMPENSATION_INTERP)
    {
        Interpolation *interpolation = &remote_object->lag_compensation.interpolation;
        InterpolationState *interp_state =
            &interpolation->state_buffer[interpolation->next_state_id % INTERPOLATION_BUFFER_SIZE];

        interp_state->tick = GameClient_GetCurrentTick();

        memcpy(&interp_state->state, state, sizeof(NetworkState));

        interpolation->next_state_id++;
    }
}

static int HandleNetworkObjectDelete(unsigned int id)
{
    if (GameClient_DeleteRemoteObject(id))
    {
        LogDebug("Deleted remote object %d", id);
    }

    return 0;
}

static bool GetInterpolationStates(
        NetworkObjectProxy *remote_object,
        unsigned int interp_tick,
        InterpolationState *state1,
        InterpolationState *state2)
{
    bool found_first_state = false;

    for (int i = 0; i < INTERPOLATION_BUFFER_SIZE; i++)
    {
        InterpolationState s = remote_object->lag_compensation.interpolation.state_buffer[i];

        if (s.tick <= interp_tick)
        {
            if (!found_first_state || s.tick > state1->tick)
            {
                memcpy(state1, &s, sizeof(InterpolationState));

                found_first_state = true;
            }
        }
    }

    if (!found_first_state)
    {
        LogWarning("Failed to find first interpolation state");

        return false;
    }

    bool found_second_state = false;

    for (int i = 0; i < INTERPOLATION_BUFFER_SIZE; i++)
    {
        InterpolationState s = remote_object->lag_compensation.interpolation.state_buffer[i];

        if (s.tick > state1->tick)
        {
            if (!found_second_state || s.tick < state2->tick)
            {
                memcpy(state2, &s, sizeof(InterpolationState));

                found_second_state = true;
            }
        }
    }

    if (!found_second_state)
        LogWarning("Failed to find second interpolation state");

    return found_second_state;
}
