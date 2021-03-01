#pragma once

#include <stdint.h>

enum
{
    INPUT_UP = 1 << 0,
    INPUT_DOWN = 1 << 1,
    INPUT_LEFT = 1 << 2,
    INPUT_RIGHT = 1 << 3,
    INPUT_SPACE = 1 << 4,
    INPUT_LEFT_2 = 1 << 5,
    INPUT_RIGHT_2 = 1 << 6
};

typedef struct
{
    uint32_t id;
    uint32_t keys;
} Input;
