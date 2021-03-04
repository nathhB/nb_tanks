#include "../scenes.h"
#include "../../common/network.h"
#include "../../common/scene_manager.h"
#include "../../common/game_object_manager.h"
#include "../game_client.h"

static int HandleMessage(void);
static int SendClientGameClockSyncMessage(void);
static void OnConnected(void);
static void OnGameClockSynchronized(void);
static void Error(void);

static bool error;

void ConnectionScene_OnEnter(void)
{
    error = false;

    LogDebug("Initialize game client");

    NBN_GameClient_Init(PROTOCOL_NAME, "127.0.0.1", PORT);
    NBN_GameClient_EnableEncryption();

    NBN_GameClient_RegisterMessage(CLIENT_GAME_CLOCK_SYNC_MESSAGE, ClientGameClockSyncMessage);
    NBN_GameClient_RegisterMessage(SERVER_GAME_CLOCK_SYNC_MESSAGE, ServerGameClockSyncMessage);
    NBN_GameClient_RegisterMessage(INPUT_MESSAGE, InputMessage);
    NBN_GameClient_RegisterMessage(GAME_SNAPSHOT_MESSAGE, GameSnapshotMessage);
    NBN_GameClient_RegisterMessage(ACK_GAME_SNAPSHOT_MESSAGE, AckGameSnapshotMessage);

    NBN_GameClient_SetPing(0.1);

    if (NBN_GameClient_Start() < 0)
    {
        LogError("Failed to start game client");
        Error();
    }
}

void ConnectionScene_OnExit(void)
{
    if (error)
    {
        NBN_GameClient_Stop();
        NBN_GameClient_Deinit();
    }
}

int ConnectionScene_OnSimulate(Input *input, double dt)
{
    if (!error)
    {
        NBN_GameClient_AddTime(dt);

        int ev;

        while ((ev = NBN_GameClient_Poll()) != NBN_NO_EVENT)
        {
            if (ev == NBN_ERROR)
            {
                LogError("Failed to connect: an error occured while poling game client events");
                Error();

                break;
            }

            if (ev == NBN_CONNECTED)
            {
                OnConnected();
            }
            else if (ev == NBN_DISCONNECTED)
            {
                LogError("Failed to connect: server cannot be reached");
                Error();

                break;
            }
            else if (ev == NBN_MESSAGE_RECEIVED)
            {
                if (HandleMessage() < 0)
                {
                    LogError("Failed to connect: an error occured while processing a received message");
                    Error();
                }
            }
        }

        if (NBN_GameClient_SendPackets() < 0)
        {
            LogError("Failed to connect: failed to send packets");
            Error();
        }
    }

    return 0;
}

int ConnectionScene_OnDraw(double alpha)
{
    return 0;
}

static int HandleMessage(void)
{
    NBN_MessageInfo msg_info = NBN_GameClient_GetReceivedMessageInfo();

    if (msg_info.type == SERVER_GAME_CLOCK_SYNC_MESSAGE)
    {
        ServerGameClockSyncMessage *msg = msg_info.data;

        unsigned int rtt = GameClient_GetCurrentTick() - msg->client_tick;
        unsigned int server_tick = msg->server_tick + (rtt / 2);

        GameClient_SyncGameClock(server_tick);
        OnGameClockSynchronized();
    }

    NBN_GameClient_DestroyMessage(msg_info.type, msg_info.data); 

    return 0;
}

static int SendClientGameClockSyncMessage(void)
{
    ClientGameClockSyncMessage *msg = NBN_GameClient_CreateReliableMessage(CLIENT_GAME_CLOCK_SYNC_MESSAGE);

    if (!msg)
        return -1;

    msg->tick = GameClient_GetCurrentTick();

    if (NBN_GameClient_SendMessage() < 0)
        return -1;

    return 0;
}

static void OnConnected(void)
{
    NBN_AcceptData *data = NBN_GameClient_GetAcceptData();
    uint32_t network_id = NBN_AcceptData_ReadUInt(data);
    Vector2 spawn_pos = (Vector2){ NBN_AcceptData_ReadUInt(data), NBN_AcceptData_ReadUInt(data) };

    LogInfo("Connected (network id: %d, spawned at (%f, %f))", network_id, spawn_pos.x, spawn_pos.y);

    GameObjectManager_Init();
    GameClient_Init(network_id, spawn_pos);

    LogInfo("Synchronizing game clock...");

    if (SendClientGameClockSyncMessage())
    {
        LogError("Failed to connect: failed to send ClientGameClockSyncMessage message");

        Error();
    }
}

static void OnGameClockSynchronized(void)
{
    LogInfo("Game clock synchronized (tick: %d)", GameClient_GetCurrentTick());

    SceneManager_ChangeScene(GAME_SCENE);
}

static void Error(void)
{
    error = true;

    SceneManager_ChangeScene(CONNECTION_ERROR_SCENE);
}
