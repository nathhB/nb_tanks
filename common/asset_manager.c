#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <raylib.h>

#include "asset_manager.h"
#include "memory_manager.h"
#include "assertion.h"

#define ASSET_MANAGER_MAX_ASSETS 255
#define TEXTURE_ATLAS_INFO_TOKENS_COUNT 6
#define TEXTURE_ATLAS_SPRITE_INFO_TOKENS_COUNT 13
#define TEXTURE_ATLAS_FILE_LINE_MAX_SIZE 255

typedef struct
{
    bool is_active;
    Asset asset;
} AssetSlot;

static int LoadTextureAsset(Asset *asset, const char *path);
static int LoadTextureAtlasAsset(Asset *asset, const char *path);
static void ParseTextureAtlasFile(FILE *file, TextureAtlas *atlas);
static char **ParseTextureAtlasFileLine(char *line, unsigned int token_count);
static void UnloadTextureAsset(Asset *asset);
static void UnloadTextureAtlasAsset(Asset *asset);
static void DestroyTextureAtlasRegion(void *region_ptr);

static AssetSlot assets[ASSET_MANAGER_MAX_ASSETS] = { { .is_active = false } };

Asset *AssetManager_LoadAsset(unsigned int tag, const char *path, AssetType type)
{
    Assert(!assets[tag].is_active, "This tag already exists");

    Asset *asset = &assets[tag].asset;

    asset->tag = tag;
    asset->type = type;

    int ret = -1;

    switch (type)
    {
        case ASSET_TEXTURE:
            ret = LoadTextureAsset(asset, path);
            break;

        case ASSET_TEXTURE_ATLAS:
            ret = LoadTextureAtlasAsset(asset, path);
            break;
    }

    Assert(ret == 0, "Failed to load asset");

    assets[tag].is_active = true;

    LogInfo("Loaded asset of type %d (TAG: %d)", asset->type, asset->tag);

    return asset;
}

Asset *AssetManager_GetAsset(unsigned int tag)
{
    Assert(assets[tag].is_active, "Asset does not exist");

    return &assets[tag].asset;
}

void AssetManager_UnloadAsset(unsigned int tag)
{
    Assert(assets[tag].is_active, "Asset does not exist");

    Asset *asset = &assets[tag].asset;

    switch (asset->type)
    {
        case ASSET_TEXTURE:
            UnloadTextureAsset(asset);
            break;

        case ASSET_TEXTURE_ATLAS:
            UnloadTextureAtlasAsset(asset);
            break;
    }

    assets[tag].is_active = false;

    LogInfo("Unloaded asset of type %d (TAG: %d)", asset->type, tag);
}

static int LoadTextureAsset(Asset *asset, const char *path)
{
    asset->data.texture = LoadTexture(path);

    return 0;
}

static int LoadTextureAtlasAsset(Asset *asset, const char *path)
{
    FILE *atlas_file = fopen(path, "r");

    if (atlas_file == NULL)
    {
        LogError("Cannot read atlas file: %s", path);

        return -1;
    }

    asset->data.atlas.regions = HTable_Create(MEM_TEXTURE_ATLASES);

    ParseTextureAtlasFile(atlas_file, &asset->data.atlas);
    fclose(atlas_file);

    LogInfo("Done loading texture altas: %s", path);

    return 0;
}

static void ParseTextureAtlasFile(FILE *file, TextureAtlas *atlas)
{
    char line[TEXTURE_ATLAS_FILE_LINE_MAX_SIZE];

    while (fgets(line, TEXTURE_ATLAS_FILE_LINE_MAX_SIZE, file))
    {
        char line_type = line[0];
        char *line_data = line + 2; // remove line type and first white space
        char **tokens = NULL;

        if (line_type == 'a')
        {
            tokens = ParseTextureAtlasFileLine(line_data, TEXTURE_ATLAS_INFO_TOKENS_COUNT);
            char texture_path[255] = {0};

            strcat(strcat(texture_path, "client/assets/atlases/"), tokens[0]);

            LogInfo("Parsed atlas description line (texture_path: %s)", texture_path);

            atlas->texture = LoadTexture(texture_path);
        }
        else if (line_type == 's')
        {
            tokens = ParseTextureAtlasFileLine(line_data, TEXTURE_ATLAS_SPRITE_INFO_TOKENS_COUNT);
            char *region_id = strdup(tokens[0]);

            MemoryManager_TagMemoryAllocation(MEM_TEXTURE_ATLASES);

            Rectangle *region = MemoryManager_Alloc(MEM_TEXTURE_ATLASES, sizeof(Rectangle));

            region->x = (float)atoi(tokens[3]);
            region->y = (float)atoi(tokens[4]);
            region->width = (float)atoi(tokens[5]);
            region->height = (float)atoi(tokens[6]);

            HTable_Add(atlas->regions, region_id, region);

            LogInfo("Loaded atlas region (id: %s, x: %d, y: %d, w: %d, h: %d)",
                    region_id, (int)region->x, (int)region->y, (int)region->width, (int)region->height);
        }

        if (tokens)
            MemoryManager_Dealloc(MEM_TEXTURE_ATLASES, tokens);
    }
}

static char **ParseTextureAtlasFileLine(char *line, unsigned int token_count)
{
    char **tokens = MemoryManager_Alloc(MEM_TEXTURE_ATLASES, sizeof(char *) * token_count);
    char *token = strtok(line, " ");
    unsigned int token_id = 0;

    while (token != NULL) {
        tokens[token_id++] = token;
        token = strtok(NULL, " ");
    }

    return tokens;
}

static void UnloadTextureAsset(Asset *asset)
{
    UnloadTexture(asset->data.texture);
}

static void UnloadTextureAtlasAsset(Asset *asset)
{
    UnloadTexture(asset->data.atlas.texture);
    HTable_Destroy(asset->data.atlas.regions, true, DestroyTextureAtlasRegion, true);
}

static void DestroyTextureAtlasRegion(void *region_ptr)
{
    MemoryManager_Dealloc(MEM_TEXTURE_ATLASES, region_ptr);
}
