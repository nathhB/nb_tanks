#include <stdlib.h>
#include <signal.h>

#include "../common/game_loop.h"
#include "../common/network.h"
#include "../common/logging.h"
#include "../common/memory_manager.h"
#include "../common/game_object_manager.h"
#include "../common/assertion.h"
#include "game_server.h"
#include "client.h"
#include "game.h"

static void SigintHandler(int dummy);
static int SimulateTick(Input *input, double dt);
static int HandleNewConnection(void);
static void HandleClientDisconnection(void);
static int HandleClientMessage(void);
static int HandleInputMessage(InputMessage *input_message, Client *client);
static int HandleAckGameSnapshotMessage(AckGameSnapshotMessage *ack_gsnap_message, Client *client);
static int SendGameSnapshots(void);
// static void BuildPlayerNetworkState(NetworkState *state, Client *client);

static double game_time = 0;
static double last_sent_game_snapshots_time = 0;
static const double send_game_snapshots_dt = 1.0 / SEND_GAME_SNAPSHOTS_FREQUENCY;

int main(void)
{
    signal(SIGINT, SigintHandler);

    MemoryManager_Init();
    MemoryManager_RegisterTag(MEM_NBNET, "NBN");
    MemoryManager_RegisterTag(MEM_CLIENTS, "Clients");
    MemoryManager_RegisterTag(MEM_CLIENT_LIST, "Client list");

    NBN_GameServer_Init(PROTOCOL_NAME, PORT);
    NBN_GameServer_EnableEncryption();

    NBN_GameServer_RegisterMessage(INPUT_MESSAGE, InputMessage);
    NBN_GameServer_RegisterMessage(GAME_SNAPSHOT_MESSAGE, GameSnapshotMessage);
    NBN_GameServer_RegisterMessage(ACK_GAME_SNAPSHOT_MESSAGE, AckGameSnapshotMessage);

    NBN_GameServer_SetPing(0.1);

    if (NBN_GameServer_Start() < 0)
    {
        LogError("Failed to start game server");

        return 1;
    }
    
    GameServer_Init();
    GameObjectManager_Init();
    GameLoop(TICKS_PER_SECOND, SimulateTick, NULL); 

    return 0;
}

static void SigintHandler(int dummy)
{
    NBN_GameServer_Stop();
    NBN_GameServer_Deinit();
    MemoryManager_PrintReport();

    exit(0);
}

static int SimulateTick(Input *input, double dt)
{
    NBN_GameServer_AddTime(dt);

    int ev;

    while ((ev = NBN_GameServer_Poll()) != NBN_NO_EVENT)
    {
        if (ev == NBN_ERROR)
        {
            LogError("Something went wrong while polling game server events");

            return -1;
        }

        if (ev == NBN_NEW_CONNECTION)
        {
            if (HandleNewConnection() < 0)
                return -1;
        }
        else if (ev == NBN_CLIENT_DISCONNECTED)
        {
            HandleClientDisconnection();
        }
        else if (ev == NBN_CLIENT_MESSAGE_RECEIVED)
        {
            if (HandleClientMessage() < 0)
                return -1;
        }
    }

    SimulateGameTick();
    GameServer_UpdateAllNetworkStates();

    if (game_time - last_sent_game_snapshots_time >= send_game_snapshots_dt)
    {
        if (SendGameSnapshots() < 0)
        {
            LogError("Failed to send game snapshots");

            return -1;
        }

        last_sent_game_snapshots_time = game_time;
    }

    if (NBN_GameServer_SendPackets() < 0)
    {
        LogError("Failed to send packets");

        return -1;
    }

    GameServer_NextTick();
    game_time += dt;

    return 0;
}

static int HandleNewConnection(void)
{
    LogInfo("New connection");

    if (GameServer_GetClientCount() >= MAX_CLIENTS)
    {
        LogInfo("Connection rejected");

        NBN_GameServer_RejectIncomingConnectionWithCode(SERVER_FULL_CODE);

        return 0;
    }

    NBN_Connection *connection = NBN_GameServer_GetIncomingConnection();

    LogInfo("Connection accepted (ID: %d)", connection->id);

    if (GameServer_AddNewClient(connection) < 0)
    {
        LogError("Failed to create a new client");

        return -1;
    }

    Client *client = GameServer_FindClientById(connection->id);

    if (!client)
    {
        LogError("Failed to create a new client");

        return -1;
    }

    NBN_AcceptData *data = NBN_AcceptData_Create(); 

    NBN_AcceptData_WriteUInt(data, client->network_tank_object->id); // send network id of client controlled tank

    // send spawn position on connection
    NBN_AcceptData_WriteUInt(data, client->tank_object->properties.tank.position.x);
    NBN_AcceptData_WriteUInt(data, client->tank_object->properties.tank.position.y);

    NBN_GameServer_AcceptIncomingConnection(data);

    return 0;
}

static void HandleClientDisconnection(void)
{
    uint32_t client_id = NBN_GameServer_GetDisconnectedClientId();

    LogInfo("Client %d has disconnected", client_id);

    Client *client = GameServer_FindClientById(client_id);

    if (!client)
    {
        // LogWarning("An unknown client has disconnected (this should never happen)");
        return;
    }

    GameServer_RemoveClient(client);
}

static int HandleClientMessage(void)
{
    NBN_MessageInfo msg_info = NBN_GameServer_GetReceivedMessageInfo();
    Client *client = GameServer_FindClientById(msg_info.sender->id);

    if (!client)
    {
        LogWarning("Received a message for an unknown client (this should never happen)");

        NBN_GameServer_DestroyMessage(msg_info.type, msg_info.data);

        return 0;
    }

    if (msg_info.type == INPUT_MESSAGE)
    {
        if (HandleInputMessage(msg_info.data, client) < 0)
            return -1;
    }
    else if (msg_info.type == ACK_GAME_SNAPSHOT_MESSAGE)
    {
        if (HandleAckGameSnapshotMessage(msg_info.data, client) < 0)
            return -1;
    }

    NBN_GameServer_DestroyMessage(msg_info.type, msg_info.data);

    return 0;
}

static int HandleInputMessage(InputMessage *input_message, Client *client)
{
    Client_AddInput(client, &input_message->input);

    return 0;
}

static int HandleAckGameSnapshotMessage(AckGameSnapshotMessage *ack_gsnap_message, Client *client)
{
    Client_AckGameSnapshot(client, ack_gsnap_message->id);

    return 0;
}

static int SendGameSnapshots(void)
{
    ListNode *current = GameServer_GetClients()->head;

    while (current)
    {
        Client *client = current->data;

        GameSnapshot *game_snapshot = Client_CreateGameSnapshot(client);

        if (!game_snapshot)
        {
            GameServer_KickClient(client);

            return 0;
        }

        GameSnapshotMessage *msg = NBN_GameServer_CreateUnreliableMessage(GAME_SNAPSHOT_MESSAGE);

        if (!msg)
            return -1;

        memcpy(&msg->game_snapshot, game_snapshot, sizeof(GameSnapshot));

        NBN_GameServer_SendMessageTo(client->connection);

        current = current->next;
    }

    return 0;
}

/*static void BuildPlayerNetworkState(NetworkState *state, Client *client)
{
    state->id = client->tank.network_id;
    state->type = NETWORK_PLAYER;
    state->state.player.position = client->tank.position;
    state->state.player.rotation = client->tank.rotation;
    state->state.player.turret_rotation = client->tank.turret_rotation;
}*/
