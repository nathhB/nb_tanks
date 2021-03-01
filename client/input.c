#include <string.h>
#include <raylib.h>

#include "input.h"

void ReadInputsFromKeyboard(Input *input)
{
    memset(input, 0, sizeof(Input));

    if (IsKeyDown(KEY_W))
        input->keys |= INPUT_UP;

    if (IsKeyDown(KEY_S))
        input->keys |= INPUT_DOWN;

    if (IsKeyDown(KEY_A))
        input->keys |= INPUT_LEFT;

    if (IsKeyDown(KEY_D))
        input->keys |= INPUT_RIGHT;

    if (IsKeyDown(KEY_LEFT))
        input->keys |= INPUT_LEFT_2;

    if (IsKeyDown(KEY_RIGHT))
        input->keys |= INPUT_RIGHT_2;

    if (IsKeyDown(KEY_SPACE))
        input->keys |= INPUT_SPACE;
}
