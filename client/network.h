#pragma once

#include "../common/network.h"
#include "../common/game_object.h"

#define INPUT_BUFFER_SIZE 16
#define INTERPOLATION_BUFFER_SIZE 16
#define INTERPOLATION_TICKS 6

typedef enum
{
    LAG_COMPENSATION_NONE,
    LAG_COMPENSATION_INTERP
} LagCompensationPolicy;

typedef struct
{
    unsigned int tick;
    NetworkState state;
} InterpolationState;

typedef void (*InterpolationFunc)(GameObject *, unsigned int, InterpolationState *, InterpolationState *);

typedef struct
{
    InterpolationState state_buffer[INTERPOLATION_BUFFER_SIZE];
    unsigned int next_state_id;
    InterpolationFunc interpolation_func;
} Interpolation;

typedef struct __NetworkObjectProxy NetworkObjectProxy;
typedef void (*OnNetworkStateUpdateFunc)(GameObject *, NetworkState *);
typedef GameObject *(*CreateGameObjectFunc)(NetworkState *);

typedef struct
{
    OnNetworkStateUpdateFunc on_state_update;
    CreateGameObjectFunc create_game_object;
    LagCompensationPolicy lag_compensation_policy;

    union
    {
        InterpolationFunc interpolation_func;
    } lag_compensation;
} NetworkObjectProxyBlueprint;

struct __NetworkObjectProxy
{
    uint32_t id;
    NetworkObjectType type;
    GameObject *game_object;
    NetworkState state;
    OnNetworkStateUpdateFunc on_state_update;
    LagCompensationPolicy lag_compensation_policy;

    union
    {
        Interpolation interpolation;
    } lag_compensation;
};

bool GetNetworkObjectProxyBlueprint(NetworkObjectType type, NetworkObjectProxyBlueprint *blueprint);
