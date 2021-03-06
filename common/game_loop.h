#pragma once

#include "input.h"

#define TICKS_PER_SECOND 60

typedef int (*SimulateTickCallback)(Input *, double dt);
typedef int (*DrawCallback)(double alpha);

int GameLoop(unsigned int ticks_per_second, SimulateTickCallback simulate_cb, DrawCallback draw_cb);
