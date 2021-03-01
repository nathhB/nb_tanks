#define NBNET_IMPL

#include "nbnet.h"

#ifdef __EMSCRIPTEN__

#include <net_drivers/webrtc.h>

#else

#include <net_drivers/udp.h>

#endif
