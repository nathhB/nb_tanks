#include <string.h>

#include "client.h"
#include "game_server.h"
#include "../common/memory_manager.h"
#include "../common/tank.h"
#include "../common/assertion.h"

static int AddUpdateEventsToGameSnapshot(Client *client, GameSnapshot *game_snapshot);
static int AddDeleteEventsToGameSnapshot(Client *client, GameSnapshot *game_snapshot);

Client *Client_Create(NBN_Connection *connection)
{
    Client *client = MemoryManager_Alloc(MEM_CLIENTS, sizeof(Client));

    client->connection = connection;
    client->network_tank_object = NULL;
    client->tank_object = NULL;
    client->next_game_snapshot_id = 0;
    client->next_input_id = 0;
    client->next_consume_input_id = 0;
    client->last_processed_input_id = 0;
    client->last_acked_game_snapshot_id = 0;

    memset(client->input_buffer, 0, sizeof(client->input_buffer));
    memset(client->game_snapshot_buffer, 0, sizeof(client->game_snapshot_buffer));

    return client;
}

void Client_Destroy(Client *client)
{
    MemoryManager_Dealloc(MEM_CLIENTS, client);
}

void Client_AddInput(Client *client, Input *input)
{
    // LogDebug("Add input: %d", input->id);

    unsigned int slot_id = client->next_input_id % INPUT_BUFFER_SIZE;

    // LogDebug("Got input: %d (slot: %d)", input->keys, slot_id);

    memcpy(&client->input_buffer[slot_id], input, sizeof(Input));

    if (client->next_input_id == 0)
        client->next_input_id = client->next_consume_input_id;
    else
        client->next_input_id++;
}

bool Client_ConsumeNextInput(Client *client, Input *res_input)
{
    Input *input = &client->input_buffer[client->next_consume_input_id % INPUT_BUFFER_SIZE];

    // LogDebug("Consume input: %d (slot: %d)", input->keys, client->last_processed_input_id % INPUT_BUFFER_SIZE);

    if (input->id < client->last_processed_input_id)
    {
        LogWarning("Received inconsistent input from client %d", client->connection->id);

        return false;
    }

    memcpy(res_input, input, sizeof(Input));

    client->next_consume_input_id++;

    return true;
}

GameSnapshot *Client_CreateGameSnapshot(Client *client)
{
    unsigned int id = client->next_game_snapshot_id++;

    if (id - client->last_acked_game_snapshot_id >= GAME_SNAPSHOT_BUFFER_SIZE)
    {
        LogError("Failed to create game snapshot for client %d: too much unacked game snapshots", client->connection->id);

        return NULL;
    }

    GameSnapshot *game_snapshot = &client->game_snapshot_buffer[id % GAME_SNAPSHOT_BUFFER_SIZE];

    GameSnapshot_Init(game_snapshot, id);

    game_snapshot->last_processed_input_id = client->last_processed_input_id; 

    if (AddUpdateEventsToGameSnapshot(client, game_snapshot) < 0)
    {
        LogError("An error occured while adding update events to game snapshot");

        return NULL;
    }

    if (AddDeleteEventsToGameSnapshot(client, game_snapshot) < 0)
    {
        LogError("An error occured while adding delete events to game snapshot");

        return NULL;
    }

    return game_snapshot;
}

void Client_AckGameSnapshot(Client *client, unsigned int id)
{
    if (id <= client->last_acked_game_snapshot_id)
        return;

    GameSnapshot *last_acked_game_snapshot = &client->game_snapshot_buffer[id % GAME_SNAPSHOT_BUFFER_SIZE];

    Assert(last_acked_game_snapshot->id == id, "invalid game snapshot id");
    
    for (unsigned int i = 0; i < last_acked_game_snapshot->event_count; i++)
    {
        NetworkEvent *network_event = &last_acked_game_snapshot->events[i];

        if (network_event->type == NETWORK_EV_DELETE)
            continue;

        NetworkObject *network_object = GameServer_FindNetworkObjectById(network_event->network_id);

        if (!network_object)
            continue;

        network_object->has_been_acked_once = true;
        
        memcpy(&network_object->last_acked_state, &network_event->state, sizeof(NetworkState));
    }

    client->last_acked_game_snapshot_id = id;
}

static int AddUpdateEventsToGameSnapshot(Client *client, GameSnapshot *game_snapshot)
{
    NetworkObject *network_objects[MAX_NETWORK_OBJECTS];
    unsigned int object_count = GameServer_GetNetworkObjects(network_objects);

    for (unsigned int i = 0; i < object_count; i++)
    {
        NetworkObject *network_object = network_objects[i];
        bool is_update_needed =
            !network_object->has_been_acked_once || network_object->is_snapshot_update_needed(network_object);

        if (is_update_needed)
        {
            if (GameSnapshot_AddEvent(game_snapshot, NETWORK_EV_UPDATE, network_object) < 0)
                return -1;
        }
    }

    return 0;
}

static int AddDeleteEventsToGameSnapshot(Client *client, GameSnapshot *game_snapshot)
{
    // loop through all the last acked game snapshot's network objects and create a delete 
    // network event for all the ones that no longer exist
    
    GameSnapshot *last_acked_game_snapshot = &client->game_snapshot_buffer[
        client->last_acked_game_snapshot_id % GAME_SNAPSHOT_BUFFER_SIZE
    ]; 

    Assert(
            last_acked_game_snapshot->id == client->last_acked_game_snapshot_id,
            "invalid game snapshot id"
          );

    for (unsigned int i = 0; i < last_acked_game_snapshot->event_count; i++)
    {
        NetworkEvent *ev = &last_acked_game_snapshot->events[i];

        if (ev->type == NETWORK_EV_UPDATE && !GameServer_FindNetworkObjectById(ev->network_id))
        {
            NetworkObject network_object = { .id = ev->network_id, .type = ev->obj_type };

            if (GameSnapshot_AddEvent(game_snapshot, NETWORK_EV_DELETE, &network_object) < 0)
                return -1;
        }
    }

    return 0;
}
