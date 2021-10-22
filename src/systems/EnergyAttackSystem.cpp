
#include "EnergyAttackSystem.h"

#include "core/system.h"

#include "core/coordinator.h"

#include <iostream>

#include <cmath>

#define PI 3.14159265

extern Coordinator gCoordinator;

void EnergyAttackSystem::Init()
{
	//initialize 40 beams available on screen
	for(size_t i = 0; i < MAX_ENERGY_BEAMS_ON_SCREEN; i++)
	{
		queue_available_pool_energy_beam_indices.emplace(i);
	}
	
}

void EnergyAttackSystem::HandleEnergyBeamActivation()
{
	for (auto const& entity : mEntities)
	{
		auto& energy_attacker = gCoordinator.GetComponent<EnergyAttacker>(entity);
		auto& transform = gCoordinator.GetComponent<Transform2D>(entity);
		
		//if queue of energy beams available from pool is not empty
		if(!queue_available_pool_energy_beam_indices.empty() && energy_attacker.send_energy_beam)
		{
			//get avaiable energy beam from pool
			uint8_t front_index = queue_available_pool_energy_beam_indices.front();
			
			if(energy_attacker.energy_index_available < 4)
			{
				energy_attacker.energy_index_available++;
				energy_attacker.energy_beam_use[energy_attacker.energy_index_available] = true;
			}
			else
			{
				energy_attacker.energy_index_available = -1;
				continue;
			}
			
			
			//set small energy beam collision, start, end, and reference to energy beam pool index
			SmallEnergyBeam& beam = small_energy_beams[front_index];
			beam.collision_rect = {transform.position.x, transform.position.y, 30.0f,30.0f};			
			beam.start_point = {transform.position.x, transform.position.y};
			
			float rad_angle = energy_attacker.energy_beam_angle_deg * ( PI / 180.0f);
			beam.end_point = {transform.position.x + cos(rad_angle)*640.0f, transform.position.y + sin(rad_angle)*360.0f};
			
			
			beam.energy_beam_attacker_index = energy_attacker.current_index_active;
			
			float slope_step = 2.0f;
			
			beam.projectile_speed_x = (beam.end_point.x - beam.start_point.x) / slope_step;
			
			beam.projectile_speed_y = (beam.end_point.y - beam.start_point.y) / slope_step;
			
			beam.active = true;
			
			if(energy_attacker.current_index_active < 4)
			{
				energy_attacker.pool_energy_indices_active[energy_attacker.current_index_active] = front_index;
				energy_attacker.current_index_active++;
			}
			
			
			//take off 1 energy beam from available pool
			queue_available_pool_energy_beam_indices.pop();
			
			energy_attacker.send_energy_beam = false;
		}
		
	}
}

void EnergyAttackSystem::HandleEnergyBeamMovement(float& dt)
{
	for( auto& beam : small_energy_beams)
	{
		if(beam.active)
		{
			
			//move beam
			beam.collision_rect.x += beam.projectile_speed_x*dt;
			beam.collision_rect.y += beam.projectile_speed_y*dt;
		}
	}
}

void EnergyAttackSystem::HandleCollisionWithWorldTiles()
{
	
}

void EnergyAttackSystem::HandleCollisionWithGeneralActors()
{
	
}

void EnergyAttackSystem::RenderEnergyBeams_FreeplayMode(CameraManager* camera_manager_ptr)
{
	for( auto& beam : small_energy_beams)
	{
		if(beam.active)
		{
			for(size_t i = 0; i < camera_manager_ptr->screens.size(); i++)
			{
				if(camera_manager_ptr->screens[i].in_active_use)
				{
					if(camera_manager_ptr->screens[i].camera_ptr)
					{
						
						DrawRectangle(beam.collision_rect.x - camera_manager_ptr->screens[i].camera_ptr->GetCameraRectPointer()->x, 
									beam.collision_rect.y - camera_manager_ptr->screens[i].camera_ptr->GetCameraRectPointer()->y, 
										beam.collision_rect.width, beam.collision_rect.height, 
										RED);
						
					}
					else
					{
						std::cout << "Uninitialized camera for active screen!\n";
					}
				}
				
			}
			
		}
	}
}
