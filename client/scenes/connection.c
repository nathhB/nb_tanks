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
static bool is_connected;

void ConnectionScene_OnEnter(void)
{
    error = false;
    is_connected = false;

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

        if (is_connected)
            GameClient_NextTick(); // once connected, start updating the clock for synchronization
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
        unsigned int one_way = rtt / 2 + rtt % 2;
        unsigned int server_tick = msg->server_tick + one_way;
        LogDebug("RTT: %d | One way: %d", rtt, one_way);

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

    LogInfo("Connected (network id: %d)", network_id);

    GameObjectManager_Init();
    GameClient_Init(network_id);

    LogInfo("Synchronizing game clock...");

    if (SendClientGameClockSyncMessage())
    {
        LogError("Failed to connect: failed to send ClientGameClockSyncMessage message");

        Error();
    }

    is_connected = true;
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
