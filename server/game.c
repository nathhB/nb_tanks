#include "../common/list.h"
#include "../common/logging.h"
#include "../common/game_object_manager.h"
#include "game.h"
#include "client.h"
#include "game_server.h"

int SimulateGameTick(void)
{
    ListNode *current = GameServer_GetClients()->head;

    while (current)
    {
        Client *client = current->data;
        Input client_input;

        Client_ConsumeNextInput(client, &client_input);

        current = current->next;
    }

    if (GameObjectManager_UpdateGameObjects(GameServer_GetCurrentTick()) < 0)
        return -1;

    return 0;
}
