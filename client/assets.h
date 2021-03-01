#pragma once

#include "../common/asset_manager.h"
#include "../common/assertion.h"
#include "rendering.h"

enum
{
    ATLAS_ASSET
};

static inline Sprite GetSprite(unsigned int tag, const char *name)
{
    Asset *asset = AssetManager_GetAsset(tag);
    Assert(asset->type == ASSET_TEXTURE_ATLAS, "Asset is not a texture atlas");

    Rectangle *region = HTable_Get(asset->data.atlas.regions, name);

    Assert(region, "Could not find region from atlas");

    return (Sprite){ asset->data.atlas.texture, *region };
}
