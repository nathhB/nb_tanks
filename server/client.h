#pragma once

#include <stdbool.h>

#include "../common/nbnet.h"
#include "../common/input.h"
#include "network.h"

typedef struct
{
    NBN_Connection *connection;
    NetworkObject *network_tank_object;
    Input input_buffer[INPUT_BUFFER_SIZE];
    Input current_input;
    GameSnapshot game_snapshot_buffer[GAME_SNAPSHOT_BUFFER_SIZE];
    unsigned int next_game_snapshot_id;
    unsigned int next_input_id;
    unsigned int next_consume_input_id;
    unsigned int last_processed_client_tick;
    unsigned int last_acked_game_snapshot_id;
} Client;

Client *Client_Create(NBN_Connection *connection);
void Client_Destroy(Client *client);
void Client_AddInput(Client *client, Input *input);
bool Client_ConsumeNextInput(Client *client, Input *res_input);
GameSnapshot *Client_CreateGameSnapshot(Client *client);
void Client_AckGameSnapshot(Client *client, unsigned int id);
