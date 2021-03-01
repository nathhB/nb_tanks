#pragma once

#include <stdlib.h>

#define MEMORY_MANAGER_MAX_TAGS 255

#define Alloc(tag, size) NB_MemoryManager_Alloc(tag, size)
#define Dealloc(tag, size) NB_MemoryManager_Dealloc(tag, size)

enum
{
    MEM_NBNET,
    MEM_CLIENTS,
    MEM_CLIENT_LIST,
    MEM_TEXTURE_ATLASES
};

typedef struct
{
    const char *tags[MEMORY_MANAGER_MAX_TAGS];
    unsigned int allocs[MEMORY_MANAGER_MAX_TAGS];
    unsigned int deallocs[MEMORY_MANAGER_MAX_TAGS];
    unsigned int total_allocs;
    unsigned int total_deallocs;
} MemoryManager;

void MemoryManager_Init(void);
void MemoryManager_RegisterTag(unsigned int tag, const char *name);
void *MemoryManager_Alloc(unsigned int tag, size_t size);
void MemoryManager_Dealloc(unsigned int tag, void *ptr);
void *MemoryManager_Realloc(void *ptr, size_t size);
void MemoryManager_TagMemoryAllocation(unsigned int tag);
void MemoryManager_PrintReport(void);
