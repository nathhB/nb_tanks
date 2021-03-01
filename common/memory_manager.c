#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_manager.h"
#include "assertion.h"

static MemoryManager memory_manager;

void MemoryManager_Init(void)
{
    memset(&memory_manager, 0, sizeof(MemoryManager));
}

void MemoryManager_RegisterTag(unsigned int tag, const char *name)
{
    Assert(memory_manager.tags[tag] == NULL, "Cannot register tag: tag already exists");

    memory_manager.tags[tag] = name;
}

void *MemoryManager_Alloc(unsigned int tag, size_t size)
{
    Assert(memory_manager.tags[tag] != NULL, "Cannot allocate memory: tag does not exist");

    void *ptr = malloc(size);

    Assert(ptr != NULL, "Cannot allocate memory: malloc failed");

    memory_manager.allocs[tag]++;
    memory_manager.total_allocs++;

    return ptr;
}

void MemoryManager_Dealloc(unsigned int tag, void *ptr)
{
    Assert(memory_manager.tags[tag] != NULL, "Cannot deallocate memory: tag does not exist");

    free(ptr);

    memory_manager.deallocs[tag]++;
    memory_manager.total_deallocs++;
}

void *MemoryManager_Realloc(void *ptr, size_t size)
{
    void *new_ptr = realloc(ptr, size);

    Assert(new_ptr != NULL, "Cannot reallocate: pointer is NULL");

    return new_ptr;
}

void MemoryManager_TagMemoryAllocation(unsigned int tag)
{
    Assert(memory_manager.tags[tag] != NULL, "Cannot tag memory: tag does not exist");

    memory_manager.total_allocs++;
    memory_manager.allocs[tag]++;
}

void MemoryManager_PrintReport(void)
{
    printf("--- Memory report ---\n\n");
    printf("Total allocations: %d\n", memory_manager.total_allocs);
    printf("Total deallocations: %d\n", memory_manager.total_deallocs);
    printf("\nDetails:\n\n");

    for (int i = 0; i < MEMORY_MANAGER_MAX_TAGS; i++)
    {
        if (memory_manager.allocs[i] > 0)
            printf("%s : (%d / %d)\n", memory_manager.tags[i], memory_manager.allocs[i], memory_manager.deallocs[i]);
    }
    printf("---------------------\n");
}
