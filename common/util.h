#pragma once

#include "raylib.h"

int LerpI(int start, int end, float alpha);
int LerpRotation(int start, int end, float alpha);
Vector2 AngleToDirection(int angle);