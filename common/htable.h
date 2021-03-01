#pragma once

#include <stdbool.h>

#include "list.h"

#define HTABLE_DEFAULT_INITIAL_CAPACITY 200
#define HTABLE_LOAD_FACTOR_THRESHOLD 0.75

typedef struct
{
    const char *key;
    void *item;
    unsigned int slot;
} HTableEntry;

typedef struct
{
    HTableEntry **internal_array;
    unsigned int tag;
    unsigned int capacity;
    unsigned int count;
    float load_factor;
} HTable;

typedef void (*HTableDestroyItemFunc)(void *);

HTable *HTable_Create(unsigned int tag);
HTable *HTable_CreateWithCapacity(unsigned int tag, unsigned int capacity);
void HTable_Destroy(HTable *htable, bool destroy_items, HTableDestroyItemFunc destroy_item_func, bool destroy_keys);
void HTable_Add(HTable *htable, const char *key, void *value);
void *HTable_Get(HTable *htable, const char *key);
void *HTable_Remove(HTable *htable, const char *key);
List *HTable_GetValues(HTable *htable);
void HTable_Print(HTable *htable);
