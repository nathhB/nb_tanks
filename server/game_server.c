#include "../common/memory_manager.h"
#include "tank.h"
#include "game_server.h"

static GameServer game_server;

void GameServer_Init(void)
{
    game_server.next_network_id = 0;
    game_server.game_clock.tick = 0;
    game_server.clients = List_Create(MEM_CLIENT_LIST);
    game_server.network_object_count = 0;

    memset(game_server.network_objects, 0, sizeof(game_server.network_objects));
}

void GameServer_NextTick(void)
{
    game_server.game_clock.tick++;
}

unsigned int GameServer_GetCurrentTick(void)
{
    return game_server.game_clock.tick;
}

int GameServer_AddNewClient(NBN_Connection *connection)
{
    Client *client = Client_Create(connection);

    if (!client)
        return -1;

    if (!(client->network_tank_object = GameServer_CreateNetworkObject(NETWORK_TANK)))
        return -1;

    ServerTank *serv_tank = &client->network_tank_object->game_object->properties.serv_tank;

    serv_tank->client_id = client->connection->id;
    serv_tank->tank.position = (Vector2){ 200, 200 };

    List_PushBack(game_server.clients, client);

    return 0;
}

void GameServer_RemoveClient(Client *client)
{
    GameServer_DeleteNetworkObject(client->network_tank_object->id);
    List_Remove(game_server.clients, client);
    Client_Destroy(client);
}

void GameServer_KickClient(Client *client)
{
    LogWarning("Kick client %d: FIXME, does not work", client->connection->id);

    // NBN_GameServer_CloseClient(client->connection);
}

Client *GameServer_FindClientById(uint32_t id)
{
    ListNode *current = game_server.clients->head;

    while (current)
    {
        Client *client = current->data;

        if (client->connection->id == id)
            return client;

        current = current->next;
    }

    return NULL;
}

unsigned int GameServer_GetClientCount(void)
{
    return game_server.clients->count;
}

List *GameServer_GetClients(void)
{
    return game_server.clients;
}

NetworkObject *GameServer_CreateNetworkObject(NetworkObjectType type)
{
    if (game_server.network_object_count >= MAX_NETWORK_OBJECTS)
    {
        LogError("Cannot create a new network object: the maximum number of network objects has been reached");

        return NULL;
    }

    NetworkObjectBlueprint blueprint;

    if (!GetNetworkObjectBlueprint(type, &blueprint))
    {
        LogError("Cannot create a new remote object: no blueprint exist for network objects of type %d", type);

        return NULL;
    }

    // find an available network object slot

    NetworkObject *network_object = NULL;

    for (unsigned int i = 0; i < MAX_NETWORK_OBJECTS; i++)
    {
        if (!game_server.network_objects[i].is_active)
        {
            network_object = &game_server.network_objects[i].object;
            game_server.network_objects[i].is_active = true;

            break;
        }
    }

    if (network_object == NULL)
    {
        LogError("Cannot create a new network object: failed to find an available slot");

        return NULL;
    }

    network_object->id = game_server.next_network_id++;
    network_object->type = type;
    network_object->game_object = blueprint.create_game_object();
    network_object->has_been_acked_once = false;
    network_object->is_snapshot_update_needed = blueprint.is_snapshot_update_needed;
    network_object->update_network_state = blueprint.update_network_state;

    network_object->game_object->network_id = network_object->id;

    memset(&network_object->state, 0, sizeof(network_object->state));
    memset(&network_object->state, 0, sizeof(network_object->last_acked_state));

    game_server.network_object_count++;

    LogDebug("Created network object %d of type %d", network_object->id, network_object->type);

    return network_object;
}

bool GameServer_DeleteNetworkObject(unsigned int network_id)
{
    for (unsigned int i = 0; i < MAX_NETWORK_OBJECTS; i++)
    {
        NetworkObjectSlot *slot = &game_server.network_objects[i];

        if (slot->is_active && slot->object.id == network_id)
        {
            slot->is_active = false;
            game_server.network_object_count--;

            LogDebug("Deleted network object %d", network_id);

            return true;
        }
    }

    return false;
}

unsigned int GameServer_GetNetworkObjects(NetworkObject *network_objects[MAX_NETWORK_OBJECTS])
{
    unsigned int count = 0;

    for (unsigned int i = 0; i < MAX_NETWORK_OBJECTS; i++)
    {
        if (game_server.network_objects[i].is_active)
        {
            network_objects[count] = &game_server.network_objects[i].object;
            count++;
        }
    }

    return count;
}

NetworkObject *GameServer_FindNetworkObjectById(unsigned int network_id)
{
    for (unsigned int i = 0; i < MAX_NETWORK_OBJECTS; i++)
    {
        NetworkObjectSlot *slot = &game_server.network_objects[i];

        if (slot->is_active && slot->object.id == network_id)
            return &slot->object;
    }

    return NULL;
}
