#include "../common/asset_manager.h"
#include "projectile.h"
#include "assets.h"

static void Draw(Vector2 position, int rotation, ClientProjectile *cli_tank);
static void BuildRenderingState(RenderingState *state, ClientProjectile *cli_projectile);

int ClientProjectile_Init(ClientProjectile *cli_projectile, bool is_local)
{
    Projectile_Init(&cli_projectile->projectile);

    cli_projectile->is_local = is_local;
    cli_projectile->is_catching_up = !is_local;
    cli_projectile->catchup_tick = 0;
    cli_projectile->interp_sync_tick = 0;
    cli_projectile->renderer = NULL;

    // don't display remote projectiles right away, wait for them to be synced with remote tank interpolation
    if (is_local)
    {
        if (ClientProjectile_CreateRenderers(cli_projectile) < 0)
        {
            LogError("Failed to create projectile renderer");

            return -1;
        }
    }

    return 0;
}

int ClientProjectile_CreateRenderers(ClientProjectile *cli_projectile)
{
    Renderer *renderer = CreateRenderer((RenderFunc)Draw, (BuildRenderingStateFunc)BuildRenderingState, cli_projectile);

    if (!renderer)
        return -1;

    cli_projectile->renderer = renderer;
    cli_projectile->sprite = GetSprite(ATLAS_ASSET, "bulletGreen1_outline");

    return 0;
}

static void Draw(Vector2 position, int rotation, ClientProjectile *cli_projectile)
{
    DrawTexturePro(
        cli_projectile->sprite.atlas,
        cli_projectile->sprite.region,
        (Rectangle){ position.x, position.y, cli_projectile->projectile.size.x, cli_projectile->projectile.size.y },
        (Vector2){ cli_projectile->projectile.size.x / 2, cli_projectile->projectile.size.y / 2 },
        rotation,
        WHITE);
}

static void BuildRenderingState(RenderingState *state, ClientProjectile *cli_projectile)
{
    state->position = cli_projectile->projectile.position;
    state->rotation = cli_projectile->projectile.rotation;
}
