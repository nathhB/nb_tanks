#pragma once

#include "../common/raylib.h"
#include "../common/game_clock.h"
#include "../common/tank.h"
#include "../common/network.h"
#include "network.h"
#include "tank.h"

typedef struct
{
    NetworkObjectProxy object;
    bool is_active;
} NetworkObjectProxySlot;

typedef struct
{
    GameClock game_clock;
    bool is_game_clock_synced;
    unsigned int local_tank_network_id;
    unsigned int last_sent_input_tick; // used for client side prediction
    Input input_buffer[INPUT_BUFFER_SIZE]; // used for client side prediction
    Input current_input;
    NetworkObjectProxy *remote_tank; // client controlled tank network object proxy
    NetworkObjectProxySlot remote_objects[MAX_NETWORK_OBJECTS];
    unsigned int remote_object_count;
} GameClient;

void GameClient_Init(uint32_t network_id);
void GameClient_NextTick(void);
unsigned int GameClient_GetCurrentTick(void);
unsigned int GameClient_GetLocalTankNetworkId(void);
void GameClient_SyncGameClock(unsigned int tick);
bool GameClient_IsGameClockSynchronized(void);
GameObject *GameClient_GetClientTank(void);
int GameClient_SendInputMessage(Input *input);
void GameClient_BufferInput(Input *input);
unsigned int GameClient_GetPredictionInputs(
        unsigned int last_server_processed_input_id, Input inputs[INPUT_BUFFER_SIZE]);
NetworkObjectProxy *GameClient_CreateRemoteObject(
        unsigned int network_id, NetworkObjectType type, NetworkState *state);
bool GameClient_DeleteRemoteObject(unsigned int network_id);
NetworkObjectProxy *GameClient_FindRemoteObject(unsigned int network_id);
unsigned int GameClient_GetRemoteObjects(NetworkObjectProxy *remote_objects[MAX_NETWORK_OBJECTS]);
void GameClient_InterpolateRemoteObjects(void);
int GameClient_AckGameSnapshot(GameSnapshot *game_snapshot);
int GameClient_ProcessGameSnapshot(GameSnapshot *game_snapshot);
void GameClient_SetCurrentInput(Input *input);
Input *GameClient_GetCurrentInput(void);
