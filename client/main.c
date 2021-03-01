#include <stdlib.h>
#include <raylib.h>

#include "../common/memory_manager.h"
#include "../common/scene_manager.h"
#include "../common/asset_manager.h"
#include "../common/game_loop.h"
#include "scenes.h"
#include "assets.h"

#ifdef DEBUG
static void DebugSetWindowPosition(void);
#endif

static int SimulateTick(Input *input, double dt);
static int DrawTick(double alpha);

int main(void)
{
    InitWindow(640, 480, "Tanks");

#ifdef DEBUG
    DebugSetWindowPosition();
#endif

    SetTargetFPS(60);

    MemoryManager_Init();
    MemoryManager_RegisterTag(MEM_NBNET, "NBN");
    MemoryManager_RegisterTag(MEM_TEXTURE_ATLASES, "Texture atlases");

    AssetManager_LoadTextureAtlas(ATLAS_ASSET, "client/assets/atlases/atlas.rtpa");

    SceneManager_RegisterScene(MAIN_MENU_SCENE,
            NULL, NULL, NULL, MainMenuScene_OnSimulate, MainMenuScene_OnDraw);
    SceneManager_RegisterScene(CONNECTION_SCENE,
            NULL, ConnectionScene_OnEnter, ConnectionScene_OnExit, ConnectionScene_OnSimulate, ConnectionScene_OnDraw);
    SceneManager_RegisterScene(CONNECTION_ERROR_SCENE,
            NULL, NULL, NULL, ConnectionErrorScene_OnSimulate, ConnectionErrorScene_OnDraw);
    SceneManager_RegisterScene(GAME_SCENE,
            NULL, GameScene_OnEnter, GameScene_OnExit, GameScene_OnSimulate, GameScene_OnDraw);

    GameLoop(TICKS_PER_SECOND, SimulateTick, DrawTick);

    AssetManager_UnloadAsset(ATLAS_ASSET);

    CloseWindow();
    MemoryManager_PrintReport();

    return 0;
}

static void DebugSetWindowPosition(void)
{
    const char *cli = getenv("CLIENT");

    if (cli)
    {
        int cli_id = atoi(cli);

        if (cli_id == 1)
            SetWindowPosition(0, 200);
        else if (cli_id == 2)
            SetWindowPosition(800, 200);
    }
}

static int SimulateTick(Input *input, double dt)
{
    return SceneManager_GetCurrentScene()->on_simulate(input, dt);
}

static int DrawTick(double alpha)
{
    return SceneManager_GetCurrentScene()->on_draw(alpha);
}
