#include "../scenes.h"
#include "../../common/input.h"
#include "../../common/scene_manager.h"

int MainMenuScene_OnSimulate(Input *input, double dt)
{
    if ((input->keys & INPUT_SPACE) == INPUT_SPACE)
    {
        SceneManager_ChangeScene(CONNECTION_SCENE);
    }

    return 0;
}

int MainMenuScene_OnDraw(double alpha)
{
    return 0;
}
