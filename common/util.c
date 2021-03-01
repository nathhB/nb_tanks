#include "util.h"
#include "assertion.h"

int LerpI(int start, int end, float alpha)
{
    return start + ((end - start) * alpha);
}

int LerpRotation(int start, int end, float alpha)
{
    Assert(start >= 0 && start <= 360, "start must be a value between 0 and 360");
    Assert(end >= 0 && end <= 360, "end must be a value between 0 and 360");

    int delta;

    if (end < start && start - end >= 180)
        delta = (360 + end) - start;
    else if (start < end && end - start >= 180)
        delta = end - (360 + start);
    else
        delta = end - start;

    int angle = (int)(start + delta * alpha) % 360;

    if (angle < 0)
        angle = angle + 360;

    Assert(angle >= 0 && angle <= 360, "resulting angle should a value between 0 and 360");

    return angle;
}

Vector2 AngleToDirection(int angle)
{
    float rad = (angle - 90) * (PI / 180);

    return (Vector2){ cos(rad), sin(rad) };
}
