cmake_minimum_required(VERSION 3.0)

project(nb_tanks C)

set(COMMON_SOURCES
    common/logging.c
    common/game_loop.c
    common/scene_manager.c
    common/memory_manager.c
    common/list.c
    common/htable.c
    common/nbnet_impl.c
    common/tank.c
    common/projectile.c
    common/network.c
    common/util.c
    common/game_object_manager.c
    common/asset_manager.c)

set(CLIENT_SOURCES
    client/main.c
    client/input.c
    client/game_client.c
    client/tank.c
    client/projectile.c
    client/rendering.c
    client/network.c
    client/scenes/main_menu.c
    client/scenes/connection.c
    client/scenes/connection_error.c
    client/scenes/game.c)

set(SERVER_SOURCES
    server/main.c
    server/input.c
    server/client.c
    server/game.c
    server/game_server.c
    server/network.c)

add_compile_options(-Wall -Wextra -Wno-unused-parameter -Wno-unknown-pragmas -Wno-tautological-constant-out-of-range-compare)

add_executable(client ${CLIENT_SOURCES} ${COMMON_SOURCES})
add_executable(server ${SERVER_SOURCES} ${COMMON_SOURCES})

find_library(RAYLIB_LIBRARY raylib HINTS ${RAYLIB_LIBRARY_PATH} REQUIRED)

message("Found raylib library: ${RAYLIB_LIBRARY}")

target_link_libraries(client ${RAYLIB_LIBRARY} pthread m)
target_link_libraries(server ${RAYLIB_LIBRARY} pthread m)

if (APPLE)
  target_link_libraries(client "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio -framework CoreVideo")
  target_link_libraries(server "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio -framework CoreVideo")
endif (APPLE)

if (WIN32)
  target_link_libraries(client wsock32 ws2_32 opengl32 gdi32 winmm)
  target_link_libraries(server wsock32 ws2_32 opengl32 gdi32 winmm)
endif (WIN32)

target_include_directories(client PUBLIC "${RAYLIB_INCLUDE_PATH}")
target_include_directories(client PUBLIC "${NBNET_INCLUDE_PATH}")
target_include_directories(server PUBLIC "${RAYLIB_INCLUDE_PATH}") # for raymath.h
target_include_directories(server PUBLIC "${NBNET_INCLUDE_PATH}")

if (CMAKE_BUILD_TYPE MATCHES Debug)
  message("Compile with nbnet debug mode and packet simulator")

  target_compile_definitions(client PRIVATE DEBUG NBN_DEBUG NBN_USE_PACKET_SIMULATOR)
  target_compile_definitions(server PRIVATE DEBUG NBN_DEBUG NBN_USE_PACKET_SIMULATOR)
endif (CMAKE_BUILD_TYPE MATCHES Debug)

target_compile_definitions(client PRIVATE NB_TANKS_CLIENT)
target_compile_definitions(server PRIVATE NB_TANKS_SERVER RAYMATH_STANDALONE)
