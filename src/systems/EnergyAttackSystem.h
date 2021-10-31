#ifndef ENERGY_ATTACK_SYSTEM_H
#define ENERGY_ATTACK_SYSTEM_H

#include <core/system.h>

#include "raylib.h"

#include "misc/camera.h"

#include <queue>
#include <array>

struct SmallEnergyBeam
{
	bool active = false;
	
	Rectangle collision_rect;
	Vector2 start_point;
	Vector2 end_point;
	
	//the id of beam that energy attacker is using
	int8_t energy_beam_attacker_index = -1;
	
	float projectile_speed_x;
	float projectile_speed_y;
	
	float time_active;
	
};

struct LargeEnergyBlast
{
	bool active = false;
	
	Rectangle collision_rect;	
	Vector2 start_point;
	Vector2 end_point;
	
	//the id of beam that energy attacker is using
	int8_t energy_beam_attacker_index = -1;
	
	float projectile_speed_x;
	float projectile_speed_y;
	
	float time_active;
	
	EntityState* entity_state_ptr = nullptr;
	
	FaceDirection face_dir;
	
	//line ending to show ending of beam rectangle
	Rectangle line_end;
	
	//for beam struggle
	bool in_beam_struggle;
};


#define MAX_ENERGY_BEAMS_PER_ATTACKER 5
#define MAX_NUM_ATTACKERS 12

class EnergyAttackSystem : public System
{

public:

//function to initialize energy beams
void Init();

//function to handle activation of energy beam
void HandleEnergyBeamActivation();

//function to handle energy beam movement
void HandleEnergyBeamMovement(float& dt);

//function to handle collision against other energy projectiles
void HandleEnergyToEnergyCollisions(float& dt);

//function to handle collision against tile
void HandleCollisionWithWorldTiles();

//function to handle collision against player or enemies
void HandleCollisionWithGeneralActors();

//function to draw energy beams
void RenderEnergyBeams_VersusMode(CustomCamera* camera_ptr);
void RenderEnergyBeams_FreeplayMode(CameraManager& camera_manager_ptr);

private:

//queue of energy beams available in pool
std::array <int8_t,MAX_NUM_ATTACKERS> queue_energy_pool_available_array;

//vector to hold energy projectiles on screen
std::vector <SmallEnergyBeam> energy_pool_vector;

//vector to hold large energy blasts
std::vector <LargeEnergyBlast> large_energy_pool_vector;


//function to add energy from energy pool
void ActivateInSmallEnergyPool(size_t& index, bool& activated);
void ActivateInLargeEnergyPool(size_t& index, bool& activated);

//function to remove energy from energy pool
void DeactivateInSmallEnergyPool(size_t& iterator);
void DeactivateInLargeEnergyPool(size_t& iterator);

//for beam struggle
//indicates if energy attacker has pressed button
std::array <bool,MAX_NUM_ATTACKERS> energy_attacker_energy_button_pressed;

//beam struggle
void HandleBeamStruggleBetweenTwoBeams(LargeEnergyBlast& blast_one, LargeEnergyBlast& blast_two, float& dt);

//function to move blast
void MoveBlast(LargeEnergyBlast& blast, float& dt);

void MoveBlastBeamStruggle(LargeEnergyBlast& blast, float& dt);
void MoveBlastBackBeamStruggle(LargeEnergyBlast& blast, float& dt);

};
 

#endif
