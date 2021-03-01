#include "../common/list.h"
#include "../common/logging.h"
#include "../common/game_object_manager.h"
#include "game.h"
#include "client.h"
#include "game_server.h"

static int SimulateClient(Client *client, Input *input);

int SimulateGameTick(void)
{
    ListNode *current = GameServer_GetClients()->head;

    while (current)
    {
        Client *client = current->data;
        Input client_input;

        if (Client_ConsumeNextInput(client, &client_input))
            SimulateClient(client, &client_input);

        current = current->next;
    }

    GameObjectManager_UpdateGameObjects();

    return 0;
}

static int SimulateClient(Client *client, Input *input)
{
    Tank_ProcessInputs(&client->tank_object->properties.tank, input, GameServer_GetCurrentTick());
    // LogDebug("%f %f", client->tank.position.x, client->tank.position.y);
    // LogDebug("Process input: %d", input->id);
    client->last_processed_input_id = input->id;

    return 0;
}
