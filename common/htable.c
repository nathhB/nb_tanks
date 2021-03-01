#include <stdio.h>
#include <string.h>
#include <math.h>

#include "htable.h"
#include "memory_manager.h"

static void InsertEntry(HTable *, HTableEntry *);
static void RemoveEntry(HTable *, HTableEntry *);
static unsigned int FindFreeSlot(HTable *, HTableEntry *, bool *);
static HTableEntry *FindEntry(HTable *, const char *);
static void Grow(HTable *);
static unsigned long HashSDBM(const char *);

HTable *HTable_Create(unsigned int tag)
{
    return HTable_CreateWithCapacity(tag, HTABLE_DEFAULT_INITIAL_CAPACITY);
}

HTable *HTable_CreateWithCapacity(unsigned int tag, unsigned int capacity)
{
    HTable *htable = MemoryManager_Alloc(tag, sizeof(HTable));

    htable->tag = tag;
    htable->internal_array = MemoryManager_Alloc(tag, sizeof(HTableEntry *) * capacity);
    htable->capacity = capacity;
    htable->count = 0;
    htable->load_factor = 0;

    for (unsigned int i = 0; i < htable->capacity; i++)
        htable->internal_array[i] = NULL;

    return htable;
}

void HTable_Destroy(
        HTable *htable, bool destroy_items, HTableDestroyItemFunc destroy_item_func, bool destroy_keys)
{
    for (unsigned int i = 0; i < htable->capacity; i++)
    {
        HTableEntry *entry = htable->internal_array[i];

        if (entry)
        {
            if (destroy_items)
                destroy_item_func(entry->item);

            if (destroy_keys)
                MemoryManager_Dealloc(htable->tag, (void*)entry->key);

            MemoryManager_Dealloc(htable->tag, entry);
        }
    }

    MemoryManager_Dealloc(htable->tag, htable->internal_array);
    MemoryManager_Dealloc(htable->tag, htable);
}

void HTable_Add(HTable *htable, const char *key, void *item)
{
    HTableEntry *entry = MemoryManager_Alloc(htable->tag, sizeof(HTableEntry));

    entry->key = key;
    entry->item = item;

    InsertEntry(htable, entry);

    if (htable->load_factor >= HTABLE_LOAD_FACTOR_THRESHOLD)
        Grow(htable);
}

void *HTable_Get(HTable *htable, const char *key)
{
    HTableEntry *entry = FindEntry(htable, key);

    return entry ? entry->item : NULL;
}

void *HTable_Remove(HTable *htable, const char *key)
{
    HTableEntry *entry = FindEntry(htable, key);

    if (entry)
    {
        void *item = entry->item;

        RemoveEntry(htable, entry);

        return item;
    }

    return NULL;
}

List *HTable_GetValues(HTable *htable)
{
    List *values = List_Create(htable->tag);

    for (unsigned int i = 0; i < htable->capacity; i++)
    {
        HTableEntry *entry = htable->internal_array[i];

        if (entry)
            List_PushBack(values, entry->item);
    }

    return values;
}

void HTable_Print(HTable *htable)
{
    printf("Entry count: %d\n", htable->count);
    printf("Entries:\n");

    for (unsigned int i = 0; i < htable->capacity; i++)
    {
        HTableEntry *entry = htable->internal_array[i];

        if (entry)
            printf("\t- %s => ...\n", entry->key);
    }
}

static void InsertEntry(HTable *htable, HTableEntry *entry)
{
    bool use_existing_slot = false;
    unsigned int slot = FindFreeSlot(htable, entry, &use_existing_slot);

    entry->slot = slot;
    htable->internal_array[slot] = entry;

    if (!use_existing_slot)
    {
        htable->count++;
        htable->load_factor = (float)htable->count / htable->capacity;
    }
}

static void RemoveEntry(HTable *htable, HTableEntry *entry)
{
    htable->internal_array[entry->slot] = NULL;

    MemoryManager_Dealloc(htable->tag, entry);

    htable->count--;
    htable->load_factor = htable->count / htable->capacity;
}

static unsigned int FindFreeSlot(HTable *htable, HTableEntry *entry, bool *use_existing_slot)
{
    unsigned long hash = HashSDBM(entry->key);
    unsigned int slot;

    // quadratic probing

    HTableEntry *current_entry;
    unsigned int i = 0;

    do
    {
        slot = (hash + (int)pow(i, 2)) % htable->capacity;
        current_entry = htable->internal_array[slot];

        i++;
    } while (current_entry != NULL && current_entry->key != entry->key);

    if (current_entry != NULL) // it means the current entry as the same key as the inserted entry
    {
        *use_existing_slot = true;

        MemoryManager_Dealloc(htable->tag, current_entry);
    }
    
    return slot;
}

static HTableEntry *FindEntry(HTable *htable, const char *key)
{
    unsigned long hash = HashSDBM(key);
    unsigned int slot;

    //quadratic probing

    HTableEntry *current_entry;
    unsigned int i = 0;

    do
    {
        slot = (hash + (int)pow(i, 2)) % htable->capacity;
        current_entry = htable->internal_array[slot];

        if (current_entry != NULL && strcmp(current_entry->key, key) == 0)
        {
            return current_entry;
        }

        i++;
    } while (i < htable->capacity);
    
    return NULL;
}

static void Grow(HTable *htable)
{
    unsigned int old_capacity = htable->capacity;
    unsigned int new_capacity = old_capacity * 2;
    HTableEntry** old_internal_array = htable->internal_array;
    HTableEntry** new_internal_array = MemoryManager_Alloc(htable->tag, sizeof(HTableEntry*) * new_capacity);

    for (unsigned int i = 0; i < new_capacity; i++)
    {
        new_internal_array[i] = NULL;
    }

    htable->internal_array = new_internal_array;
    htable->capacity = new_capacity;
    htable->count = 0;
    htable->load_factor = 0;

    // rehash

    for (unsigned int i = 0; i < old_capacity; i++)
    {
        if (old_internal_array[i])
            InsertEntry(htable, old_internal_array[i]);
    }

    MemoryManager_Dealloc(htable->tag, old_internal_array);
}

static unsigned long HashSDBM(const char *str)
{
    unsigned long hash = 0;
    int c;

    while ((c = *str++))
    {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}
