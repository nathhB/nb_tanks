#include <raymath.h>

#include "../scenes.h"
#include "../../common/network.h"
#include "../../common/scene_manager.h"
#include "../../common/game_object_manager.h"
#include "../game_client.h"

static void OnConnected(void);
static void Error(void);

static bool error;

void ConnectionScene_OnEnter(void)
{
    error = false;

    LogDebug("Initialize game client");
    NBN_GameClient_Init(PROTOCOL_NAME, "127.0.0.1", PORT);
    NBN_GameClient_EnableEncryption();

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
                break;
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

static void OnConnected(void)
{
    NBN_AcceptData *data = NBN_GameClient_GetAcceptData();
    uint32_t network_id = NBN_AcceptData_ReadUInt(data);
    Vector2 spawn_pos = (Vector2){ NBN_AcceptData_ReadUInt(data), NBN_AcceptData_ReadUInt(data) };

    LogInfo("Connected (network id: %d, spawned at (%f, %f))", network_id, spawn_pos.x, spawn_pos.y);

    GameObjectManager_Init();
    GameClient_Init(network_id, spawn_pos);
    SceneManager_ChangeScene(GAME_SCENE);
}

static void Error(void)
{
    error = true;

    SceneManager_ChangeScene(CONNECTION_ERROR_SCENE);
}
