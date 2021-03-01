#include "../scenes.h"
#include "../../common/input.h"
#include "../../common/scene_manager.h"

int ConnectionErrorScene_OnSimulate(Input *input, double dt)
{
    if ((input->keys & INPUT_SPACE) == INPUT_SPACE)
    {
        SceneManager_ChangeScene(MAIN_MENU_SCENE);
    }

    return 0;
}

int ConnectionErrorScene_OnDraw(double alpha)
{
    return 0;
}
