#include "../common/asset_manager.h"
#include "projectile.h"
#include "assets.h"

static int CreateProjectileRenderer(ClientProjectile *cli_projectile);
static void Draw(Vector2 position, int rotation, ClientProjectile *cli_tank);
static void BuildRenderingState(RenderingState *state, ClientProjectile *cli_projectile);

int ClientProjectile_Init(ClientProjectile *cli_projectile)
{
    Projectile_Init(&cli_projectile->projectile);

    cli_projectile->sprite = GetSprite(ATLAS_ASSET, "bulletGreen1_outline");

    if (CreateProjectileRenderer(cli_projectile) < 0)
    {
        LogError("Failed to create projectile renderer");

        return -1;
    }

    return 0;
}

static int CreateProjectileRenderer(ClientProjectile *cli_projectile)
{
    return CreateRenderer((RenderFunc)Draw, (BuildRenderingStateFunc)BuildRenderingState, cli_projectile);
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
