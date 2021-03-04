#pragma once

#include "memory_manager.h"
#include "logging.h"
#include "raylib.h"

#define NBN_Allocator(size) MemoryManager_Alloc(MEM_NBNET, size)
#define NBN_Deallocator(ptr) MemoryManager_Dealloc(MEM_NBNET, ptr)

#define NBN_LogInfo(...) LogInfo(__VA_ARGS__)
#define NBN_LogError(...) LogError(__VA_ARGS__)
#define NBN_LogWarning(...) LogWarning(__VA_ARGS__)
#define NBN_LogDebug(...) do {} while(0)
#define NBN_LogTrace(...) do {} while(0)
// #define NBN_LogTrace(...) LogTrace(__VA_ARGS__)

#include <nbnet.h>
