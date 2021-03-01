#pragma once

#include "../common/input.h"

enum
{
    MAIN_MENU_SCENE,
    CONNECTION_SCENE,
    CONNECTION_ERROR_SCENE,
    GAME_SCENE
};

int MainMenuScene_OnSimulate(Input *input, double dt);
int MainMenuScene_OnDraw(double alpha);

void ConnectionScene_OnEnter(void);
void ConnectionScene_OnExit(void);
int ConnectionScene_OnSimulate(Input *input, double dt);
int ConnectionScene_OnDraw(double alpha);

int ConnectionErrorScene_OnSimulate(Input *input, double dt);
int ConnectionErrorScene_OnDraw(double alpha);

void GameScene_OnEnter(void);
void GameScene_OnExit(void);
int GameScene_OnSimulate(Input *input, double dt);
int GameScene_OnDraw(double alpha);
