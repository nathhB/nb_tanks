#pragma once

#include <raylib.h>

#include "htable.h"

typedef enum
{
    ASSET_TEXTURE,
    ASSET_TEXTURE_ATLAS
} AssetType;

typedef struct
{
    Texture2D texture;
    HTable *regions;
} TextureAtlas;

typedef struct
{
    unsigned int tag;
    AssetType type;

    union
    {
        Texture2D texture;
        TextureAtlas atlas;
    } data;
} Asset;

#define AssetManager_LoadTexture(tag, path) (AssetManager_LoadAsset(tag, path, ASSET_TEXTURE))
#define AssetManager_LoadTextureAtlas(tag, path) (AssetManager_LoadAsset(tag, path, ASSET_TEXTURE_ATLAS))

Asset *AssetManager_LoadAsset(unsigned int tag, const char *path, AssetType type);
Asset *AssetManager_GetAsset(unsigned int tag);
void AssetManager_UnloadAsset(unsigned int tag);
