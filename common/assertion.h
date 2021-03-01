#pragma once

#include <stdlib.h>

#include "logging.h"

#define Assert(cond, msg) \
{ \
    if (!(cond)) \
    { \
        LogError(msg); \
        abort(); \
    } \
}
