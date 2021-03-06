#pragma once

int Projectile_Update(GameObject *game_object, unsigned int tick);
Vector2 Projectile_ComputePosition(Projectile *projectile, unsigned int ticks);
void Projectile_OnDelete(GameObject *game_object);
