#include "tank.h"
#include "util.h"

void Tank_Init(Tank *tank)
{
    tank->position = Vector2Zero();
    tank->move_speed = 1.5f;
    tank->rotation_speed = 1.5f;
    tank->turret_rotation_speed = 1.5f;
    tank->rotation = 90;
    tank->turret_rotation = 90;
    tank->last_shoot_tick = 0;
    tank->shoot_rate = 1; 
    tank->direction = AngleToDirection(tank->rotation);
    tank->turret_direction = AngleToDirection(tank->turret_rotation);
}
