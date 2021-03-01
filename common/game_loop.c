#include <stdbool.h>
#include <time.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif /* __EMSCRIPTEN__ */

#if defined(_WIN32) || defined(_WIN64)
#include <synchapi.h> /* Sleep */
#endif /* _WIN32 || _WIN64 */

#include "game_loop.h"
#include "logging.h"

#ifdef NB_TANKS_CLIENT

#include <raylib.h>

#include "../client/input.h"

static int Draw(DrawCallback draw_cb, double alpha);

#endif /* NB_TANKS_CLIENT */

#ifdef NB_TANKS_SERVER

#include "../server/input.h"

static void ServerSleep(double secs);

#endif /* NB_TANKS_SERVER */

static double GetElapsedTime(void);

int GameLoop(unsigned int ticks_per_second, SimulateTickCallback simulate_cb, DrawCallback draw_cb)
{
    double dt = 1.0 / ticks_per_second;
    double acc = 0;
    double time = GetElapsedTime();
    bool error = false;

    (void)draw_cb;

    LogInfo("Start game loop (dt: %f)", dt);

#ifdef NB_TANKS_CLIENT
    while (!WindowShouldClose() && !error)
#elif NB_TANKS_SERVER
    while (!error)
#endif
    {
        double elapsed_time = GetElapsedTime() - time;

        time = GetElapsedTime();

        if (elapsed_time > 0.25)
            elapsed_time = 0.25;

        acc += elapsed_time;

        if (elapsed_time > dt)
            LogWarning("Simulation is running slow (%f > %f)", elapsed_time, dt);

        while (acc >= dt)
        {
            Input input;

#ifdef NB_TANKS_CLIENT
            ReadInputsFromKeyboard(&input);
#endif /* NB_TANKS_CLIENT */

            if (simulate_cb(&input, dt) < 0)
            {
                LogError("Error returned from the simulation callback");

                error = true;
                break;
            }

            acc -= dt;
        }

        if (error)
            break;

#ifdef NB_TANKS_CLIENT
        if (Draw(draw_cb, acc / dt) < 0)
        {
            error = true;
            break;
        }
#endif /* NB_TANKS_CLIENT */

#ifdef NB_TANKS_SERVER
        ServerSleep(0.01);
#endif /* NB_TANKS_SERVER */
    }

    if (error)
        LogError("Game loop exited due to an error");

    return (error) ? 1 : 0;
}

#ifdef NB_TANKS_CLIENT

static int Draw(DrawCallback draw_cb, double alpha)
{
    BeginDrawing();
    ClearBackground(BLACK);

    if (draw_cb(alpha) < 0)
    {
        LogError("Error returned from the draw callback");

        return -1;
    }

    EndDrawing();

    return 0;
}

#endif /* NB_TANKS_CLIENT */

static double GetElapsedTime(void)
{
#ifdef __EMSCRIPTEN__
    // TODO
#else
    struct timespec t;

    clock_gettime(CLOCK_REALTIME, &t);

    double time = t.tv_sec + t.tv_nsec / 1e9;

    return time;
#endif /* __EMSCRIPTEN__ */
}

#ifdef NB_TANKS_SERVER

static void ServerSleep(double secs)
{
#if defined(__EMSCRIPTEN__)
    emscripten_sleep(secs * 1000);
#elif defined(_WIN32) || defined(_WIN64)
    Sleep(secs * 1000);
#else
    long nanos = secs * 1e9;
    struct timespec t = {.tv_sec = nanos / 999999999, .tv_nsec = nanos % 999999999};

    nanosleep(&t, &t);
#endif
}

#endif /* NB_TANKS_SERVER*/
