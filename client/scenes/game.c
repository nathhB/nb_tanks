#include <string.h>

#include "../../common/network.h"
#include "../../common/scene_manager.h"
#include "../../common/assertion.h"
#include "../../common/util.h"
#include "../scenes.h"
#include "../game_client.h"
#include "../tank.h"
#include "../rendering.h"

static void SimulateGameTick(Input *input);
static int HandleMessage(void);
static int HandleGameSnapshot(GameSnapshot *game_snapshot);
static int RunClientSidePrediction(unsigned int last_server_processed_input_id);
static int PredictInput(Input *input);
static void OnDisconnected(int code);

static bool disconnected;

void GameScene_OnEnter(void)
{
    disconnected = false;
}

void GameScene_OnExit(void)
{
    NBN_GameClient_Stop();
    NBN_GameClient_Deinit();
}

int GameScene_OnSimulate(Input *input, double dt)
{
    if (disconnected)
        return 0;

    NBN_GameClient_AddTime(dt);

    int ev;

    while ((ev = NBN_GameClient_Poll()) != NBN_NO_EVENT)
    {
        if (ev == NBN_ERROR)
        {
            LogError("Disconnected: an error occured while poling game client events");
            OnDisconnected(-1);

            break;
        }

        if (ev == NBN_DISCONNECTED)
        {
            int code = NBN_GameClient_GetServerCloseCode();

            LogError("Disconnected (code: %d)", code);
            OnDisconnected(code);
        }
        else if (ev == NBN_MESSAGE_RECEIVED)
        {
            if (HandleMessage() < 0)
            {
                LogError("Disconnected: an error occured while processing a server message");
                OnDisconnected(-1);
            }
        }
    }

    if (disconnected)
        return 0;

    input->id = GameClient_GetCurrentTick();

    SimulateGameTick(input);
    GameClient_InterpolateRemoteObjects();

    if (GameClient_SendInputMessage(input) < 0)
    {
        LogError("Failed to send InputMessage");
        OnDisconnected(-1);

        return 0;
    } 

    GameClient_BufferInput(input);
    GameClient_NextTick();

    if (NBN_GameClient_SendPackets() < 0)
    {
        LogError("Failed to send packets");
        OnDisconnected(-1);

        return 0;
    }

    return 0;
}

int GameScene_OnDraw(double alpha)
{
    RenderAll(alpha);

    NBN_ConnectionStats stats = NBN_GameClient_GetStats();

    DrawText(TextFormat("Ping: %d ms", (int)(stats.ping * 1000)), 0, 0, 16, WHITE);
    DrawText(TextFormat("Packet loss: %d %%", (int)(stats.packet_loss * 100)), 0, 20, 16, WHITE);

    return 0;
}

static void SimulateGameTick(Input *input)
{
    Tank_ProcessInputs(&GameClient_GetClientTank()->tank, input, GameClient_GetCurrentTick());
}

static int HandleMessage(void)
{
    NBN_MessageInfo msg_info = NBN_GameClient_GetReceivedMessageInfo();

    if (msg_info.type == GAME_SNAPSHOT_MESSAGE)
    {
        GameSnapshot *game_snapshot = &((GameSnapshotMessage *)msg_info.data)->game_snapshot;

        if (HandleGameSnapshot(game_snapshot) < 0)
            return -1;
    }

    NBN_GameClient_DestroyMessage(msg_info.type, msg_info.data); 

    return 0;
}

static int HandleGameSnapshot(GameSnapshot *game_snapshot)
{
    if (GameClient_ProcessGameSnapshot(game_snapshot) < 0)
    {
        LogError("Failed to process game snapshot");

        return -1;
    } 

    if (RunClientSidePrediction(game_snapshot->last_processed_input_id) < 0)
        LogWarning("Client side prediction failed");

    if (GameClient_AckGameSnapshot(game_snapshot) < 0)
    {
        LogError("Failed to ack game snapshot");

        return -1;
    }

    return 0;
}

static int RunClientSidePrediction(unsigned int last_server_processed_input_id)
{
    Input inputs[INPUT_BUFFER_SIZE];

    unsigned int input_count = GameClient_GetPredictionInputs(last_server_processed_input_id, inputs);

    for (unsigned int i = 0; i < input_count; i++)
    {
        if (PredictInput(&inputs[i]) < 0)
            return -1;
    }

    return 0;
}

static int PredictInput(Input *input)
{
    // only predict directional keys
    input->keys &= ~INPUT_SPACE;

    Tank_ProcessInputs(&GameClient_GetClientTank()->tank, input, 0);

    return 0;
}

static void OnDisconnected(int code)
{
    disconnected = true;

    SceneManager_ChangeScene(MAIN_MENU_SCENE);
}
