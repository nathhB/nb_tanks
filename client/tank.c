#include <raylib.h>
#include <raymath.h>

#include "../common/asset_manager.h"
#include "../common/util.h"
#include "tank.h"
#include "assets.h"

static int CreateRenderers(ClientTank *tank);
static int CreateCoreRenderer(ClientTank *tank);
static int CreateTurretRenderer(ClientTank *tank);
static void DrawCore(Vector2 position, int rotation, ClientTank *cli_tank);
static void DrawTurret(Vector2 position, int rotation, ClientTank *cli_tank);
static void BuildCoreRenderingState(RenderingState *state, ClientTank *cli_tank);
static void BuildTurretRenderingState(RenderingState *state, ClientTank *cli_tank);

int ClientTank_Init(ClientTank *cli_tank, bool is_local)
{
    Tank_Init(&cli_tank->tank);

    cli_tank->is_local = is_local;

    cli_tank->core_sprite = GetSprite(ATLAS_ASSET, "tankBody_green");
    cli_tank->turret_sprite = GetSprite(ATLAS_ASSET, "tankGreen_barrel2_outline");
    cli_tank->tank.size = (Vector2){ cli_tank->core_sprite.region.width, cli_tank->core_sprite.region.height };

    if (CreateRenderers(cli_tank) < 0)
    {
        LogError("Failed to create tank renderers");

        return -1;
    }

    return 0;
}

static int CreateRenderers(ClientTank *cli_tank)
{
    if (CreateCoreRenderer(cli_tank) < 0)
        return -1;

    if (CreateTurretRenderer(cli_tank) < 0)
        return -1;

    return 0;
}

static int CreateCoreRenderer(ClientTank *cli_tank)
{
    if (!CreateRenderer((RenderFunc)DrawCore, (BuildRenderingStateFunc)BuildCoreRenderingState, cli_tank))
        return -1;

    return 0;
}

static int CreateTurretRenderer(ClientTank *cli_tank)
{
    if (!CreateRenderer((RenderFunc)DrawTurret, (BuildRenderingStateFunc)BuildTurretRenderingState, cli_tank))
        return -1;

    return 0;
}

static void DrawCore(Vector2 position, int rotation, ClientTank *cli_tank)
{
    DrawTexturePro(
        cli_tank->core_sprite.atlas,
        cli_tank->core_sprite.region,
        (Rectangle){ position.x, position.y, cli_tank->tank.size.x, cli_tank->tank.size.y },
        (Vector2){ cli_tank->tank.size.x / 2, cli_tank->tank.size.y / 2 },
        rotation,
        WHITE);

#ifdef DEBUG
    DrawLineEx(position, Vector2Add(position, Vector2Scale(AngleToDirection(rotation), 40)), 2, BLUE);
#endif
}

static void DrawTurret(Vector2 position, int rotation, ClientTank *cli_tank)
{
    DrawTexturePro(
        cli_tank->turret_sprite.atlas,
        cli_tank->turret_sprite.region,
        (Rectangle){ position.x, position.y, cli_tank->turret_sprite.region.width, cli_tank->turret_sprite.region.height },
        (Vector2){ cli_tank->turret_sprite.region.width / 2, cli_tank->turret_sprite.region.height - 5 },
        rotation,
        WHITE);

#ifdef DEBUG
    DrawLineEx(position, Vector2Add(position, Vector2Scale(AngleToDirection(rotation), 40)), 2, RED);
#endif
}

static void BuildCoreRenderingState(RenderingState *state, ClientTank *cli_tank)
{
    state->position = cli_tank->tank.position;
    state->rotation = cli_tank->tank.rotation;
}

static void BuildTurretRenderingState(RenderingState *state, ClientTank *cli_tank)
{
    state->position = Vector2Add(cli_tank->tank.position, (Vector2){ 0, 0 });
    state->rotation = cli_tank->tank.turret_rotation;
}
