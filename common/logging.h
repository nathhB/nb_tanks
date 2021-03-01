#pragma once

#include <string.h>

typedef enum 
{
    LOG_LEVEL_WARNING,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_INFO,
    LOG_LEVEL_TRACE
} LogLevel;

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LogWarning(...) Log(LOG_LEVEL_WARNING, __FILENAME__, __LINE__, __VA_ARGS__)
#define LogDebug(...) Log(LOG_LEVEL_DEBUG, __FILENAME__, __LINE__, __VA_ARGS__)
#define LogError(...) Log(LOG_LEVEL_ERROR, __FILENAME__, __LINE__, __VA_ARGS__)
#define LogInfo(...) Log(LOG_LEVEL_INFO, __FILENAME__, __LINE__, __VA_ARGS__)
#define LogTrace(...) Log(LOG_LEVEL_TRACE, __FILENAME__, __LINE__, __VA_ARGS__)

void Log(LogLevel type, const char *file, int line, const char *format, ...);
