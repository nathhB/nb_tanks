#include <stdio.h>
#include <stdarg.h>

#include "logging.h"

void Log(LogLevel level, const char *file, int line, const char *format, ...)
{
    static const char *log_names[] = {
        "WARNING",
        "DEBUG",
        "ERROR",
        "INFO",
        "TRACE"
    };

    va_list args;

    fprintf(stdout, "[%s] %s:%d | ", log_names[level], file, line);

    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end (args);

    fprintf(stdout, "\n");
    fflush(stdout);
}
