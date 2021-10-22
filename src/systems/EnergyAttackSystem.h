#ifndef ENERGY_ATTACK_SYSTEM_H
#define ENERGY_ATTACK_SYSTEM_H

#include <core/system.h>

#include "raylib.h"

#include "misc/camera.h"

#include <queue>

struct SmallEnergyBeam
{
	bool active;
	
	Rectangle collision_rect;
	Vector2 start_point;
	Vector2 end_point;
	
	//the id of beam that energy attacker is using
	int8_t energy_beam_attacker_index = -1;
	
	float projectile_speed_x;
	float projectile_speed_y;
	
};

#define MAX_ENERGY_BEAMS_ON_SCREEN 40

class EnergyAttackSystem : public System
{

public:

//function to initialize energy beams
void Init();

//function to handle activation of energy beam
void HandleEnergyBeamActivation();

//function to handle energy beam movement
void HandleEnergyBeamMovement(float& dt);

//function to handle collision against tile
void HandleCollisionWithWorldTiles();

//function to handle collision against player or enemies
void HandleCollisionWithGeneralActors();

//function to draw energy beams
void RenderEnergyBeams_FreeplayMode(CameraManager* camera_manager_ptr);

private:

//array of energy beams
SmallEnergyBeam small_energy_beams[MAX_ENERGY_BEAMS_ON_SCREEN];

//queue of energy beams available in pool 
std::queue <uint8_t> queue_available_pool_energy_beam_indices;

};
 

#endif
