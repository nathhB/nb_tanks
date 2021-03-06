#pragma once

#include "../common/game_clock.h"
#include "../common/list.h"
#include "client.h"
#include "network.h"

typedef struct
{
    NetworkObject object;
    bool is_active;
} NetworkObjectSlot;

typedef struct
{
    uint32_t next_network_id;
    GameClock game_clock;
    List *clients;
    NetworkObjectSlot network_objects[MAX_NETWORK_OBJECTS];
    unsigned int network_object_count;
} GameServer;

void GameServer_Init(void);
void GameServer_NextTick(void);
unsigned int GameServer_GetCurrentTick(void);
int GameServer_AddNewClient(NBN_Connection *connection);
void GameServer_RemoveClient(Client *client);
void GameServer_KickClient(Client *client);
Client *GameServer_FindClientById(uint32_t id);
unsigned int GameServer_GetClientCount(void);
List *GameServer_GetClients(void);
NetworkObject *GameServer_CreateNetworkObject(NetworkObjectType type);
bool GameServer_DeleteNetworkObject(unsigned int network_id);
unsigned int GameServer_GetNetworkObjects(NetworkObject *network_objects[MAX_NETWORK_OBJECTS]);
NetworkObject *GameServer_FindNetworkObjectById(unsigned int network_id);
