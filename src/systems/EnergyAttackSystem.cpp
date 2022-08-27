
#include "EnergyAttackSystem.h"

#include "core/system.h"

#include "core/coordinator.h"

#include <iostream>

#include <cmath>

#include "misc/level_maps.h"

#include "misc/general_sounds.h" //for playing general sound for energy attacks

extern Coordinator gCoordinator;

void EnergyAttackSystem::Init()
{
	size_t iterator = 0;
	for (auto const& entity : mEntities)
	{
		auto& energy_attacker = gCoordinator.GetComponent<EnergyAttacker>(entity);
		energy_attacker.queue_id = iterator;
		
		energy_attacker_energy_button_pressed[iterator] = false;
		iterator++; 
	}
	
	//initialize queue for all attackers
	for(size_t i = 0; i < MAX_NUM_ATTACKERS; i++)
	{
		queue_energy_pool_available_array[i] = MAX_ENERGY_BEAMS_PER_ATTACKER;
		
	}
	
	//if energy pool is already already initialized, skip adding elements to energy pool
	if(energy_pool_vector.size() > 0){return;}
	
	energy_pool_vector.reserve(MAX_ENERGY_BEAMS_PER_ATTACKER*MAX_NUM_ATTACKERS + 1);
	
	for(size_t i = 0; i < MAX_ENERGY_BEAMS_PER_ATTACKER*MAX_NUM_ATTACKERS; i++)
	{
		energy_pool_vector.emplace_back(SmallEnergyBeam());
	}
	
	large_energy_pool_vector.reserve(MAX_NUM_ATTACKERS + 1);
	
	for(size_t i = 0; i < MAX_NUM_ATTACKERS; i++)
	{
		large_energy_pool_vector.emplace_back(LargeEnergyBlast());
	}
	
	
}

void EnergyAttackSystem::HandleEnergySetupActivationFromInput(float& dt)
{
	for (auto const& entity : mEntities)
	{
		auto& gen_entity_state = gCoordinator.GetComponent<GeneralEnityState>(entity);
		auto& energy_attacker = gCoordinator.GetComponent<EnergyAttacker>(entity);
		auto& rigidBody = gCoordinator.GetComponent<RigidBody2D>(entity);
		auto& animation = gCoordinator.GetComponent<Animation>(entity);
		
		bool energy_button_released = false;
		
		//reset state if other buttons pressed
		if(gen_entity_state.regularAttackButtonHeld || gen_entity_state.regularAttackButtonPressed ||
		  gen_entity_state.powerButtonPressed)
		{
			//reset energy attack parameters
			gen_entity_state.energyButtonPressed = false;
			gen_entity_state.energyButtonHeld = false;
			energy_attacker.energy_button_released = false;
			energy_attacker.state = EnergyAttackerState::IDLE;
		}
			
		//launch small energy beam if energy beam button pressed, and player is not taking damage
		if(!gen_entity_state.regularAttackButtonPressed && !gen_entity_state.regularAttackButtonHeld 
		&& gen_entity_state.energyButtonPressed && !gen_entity_state.taking_damage 
		&& !gen_entity_state.energyButtonHeld
		&& gen_entity_state.actor_state != EntityState::HURTING_KNOCKBACK
		&& gen_entity_state.actor_state != EntityState::ATTACKING_NO_MOVE)
		{			
			energy_button_released = true;
			
			energy_attacker.energy_button_released = true;
			energy_attacker_energy_button_pressed[energy_attacker.queue_id] = true;
			
			//if player is ready to send large energy blast
			if(energy_attacker.state == EnergyAttackerState::READY_TO_SEND_LARGE_BLAST)
			{
				
				//activate large energy blast
				
				gen_entity_state.time_energy_button_held = 0.0f;
				
				energy_attacker.state = EnergyAttackerState::SEND_LARGE_BLAST;
				
				float angle = 0.0f;
					
				//shoot in the direction that player was facing last frame
				switch(animation.face_dir)
				{
					case FaceDirection::NONE:{ break;}
					case FaceDirection::NORTH:{ angle = 90.0f; break;}
					case FaceDirection::EAST:{ angle = 0.0f; break;}
					case FaceDirection::WEST:{ angle = 180.0f; break;}
					case FaceDirection::SOUTH:{ angle = -90.0f; break;}
				}
				
				energy_attacker.energy_beam_angle_deg = angle;
				
				gen_entity_state.energyButtonPressed = false;
			}
			else if(energy_attacker.state == EnergyAttackerState::IDLE || energy_attacker.state == EnergyAttackerState::CHARGING)
			{
				energy_attacker.state = EnergyAttackerState::SEND_PROJECTILE;
			
				gen_entity_state.energyButtonPressed = false;
				
				//if player is moving
				if(rigidBody.velocity.x != 0.0f || rigidBody.velocity.y != 0.0f)
				{
					float horiz = rigidBody.velocity.x;
					float vert = -1.0f*rigidBody.velocity.y + 1.0f;
							
					energy_attacker.energy_beam_angle_deg = atan2(vert,horiz)*(180.0f / PI);
				}
				//if player is not moving
				else
				{
					float angle = 0.0f;
					
					//shoot in the direction that player was facing last frame
					switch(animation.face_dir)
					{
						case FaceDirection::NONE:{ break;}
						case FaceDirection::NORTH:{ angle = 90.0f; break;}
						case FaceDirection::EAST:{ angle = 0.0f; break;}
						case FaceDirection::WEST:{ angle = 180.0f; break;}
						case FaceDirection::SOUTH:{ angle = -90.0f; break;}
					}
					
					energy_attacker.energy_beam_angle_deg = angle;
				}
			}
			
						
		}
		else
		{
			energy_attacker.energy_button_released = gen_entity_state.energyButtonPressed && !gen_entity_state.energyButtonHeld;
			gen_entity_state.energyButtonPressed = false;
			
		}
		
		//if energy button is held
		if(gen_entity_state.energyButtonHeld
		&& !gen_entity_state.regularAttackButtonPressed && !gen_entity_state.regularAttackButtonHeld
		&& !gen_entity_state.taking_damage 
		&& !energy_button_released
		&& gen_entity_state.actor_state != EntityState::HURTING_KNOCKBACK
		&& gen_entity_state.actor_state != EntityState::ATTACKING_NO_MOVE)
		{
						
			energy_attacker.energy_button_released = false;
			
			//if energy button held down for 1 second
			if(gen_entity_state.time_energy_button_held >= 1.5f)
			{
				energy_attacker.state = EnergyAttackerState::READY_TO_SEND_LARGE_BLAST;
			}
			else
			{
				gen_entity_state.time_energy_button_held += dt;
				energy_attacker.state = EnergyAttackerState::CHARGING;
			}
			
			float angle = 0.0f;
					
			//shoot in the direction that player was facing last frame
			switch(animation.face_dir)
			{
				case FaceDirection::NONE:{ break;}
				case FaceDirection::NORTH:{ angle = 90.0f; break;}
				case FaceDirection::EAST:{ angle = 0.0f; break;}
				case FaceDirection::WEST:{ angle = 180.0f; break;}
				case FaceDirection::SOUTH:{ angle = -90.0f; break;}
			}
			
			energy_attacker.energy_beam_angle_deg = angle;
		}
		else
		{
			gen_entity_state.energyButtonHeld = false;
			
		}
		
		//std::cout << " before handle energy beam activation: \n\tenergy attacker " << int(energy_attacker.queue_id) 
		//			<< " energy button press: " << energy_attacker_energy_button_pressed[energy_attacker.queue_id] << std::endl;
	}
}

void EnergyAttackSystem::HandleEnergyBeamActivation()
{
	for (auto const& entity : mEntities)
	{
		auto& energy_attacker = gCoordinator.GetComponent<EnergyAttacker>(entity);
		auto& transform = gCoordinator.GetComponent<Transform2D>(entity);
		auto& collisionBox = gCoordinator.GetComponent<CollisionBox>(entity);
		auto& gen_entity_state = gCoordinator.GetComponent<GeneralEnityState>(entity);
		
		//skip if not alive
		if(gen_entity_state.actor_state == EntityState::DEAD){continue;}
		
		//if energy beams available from queue and energy beam requested
		if(queue_energy_pool_available_array[energy_attacker.queue_id] != -1 
			)
		{
			//if sending energy projectile
			if(energy_attacker.state == EnergyAttackerState::SEND_PROJECTILE)
			{
				//take off 1 energy beam from available pool
				queue_energy_pool_available_array[energy_attacker.queue_id] -= 1;
				
				energy_attacker.state = EnergyAttackerState::IDLE;
				
				size_t index = 0;
				bool activated = false;
				EnergyAttackSystem::ActivateInSmallEnergyPool(index,activated);
				
				//if no more beams
				if(!activated){return;}
				
				SmallEnergyBeam& beam = energy_pool_vector[index];
				
				float rad_angle = energy_attacker.energy_beam_angle_deg * ( PI / 180.0f);
				beam.start_point = {transform.position.x + cos(rad_angle)*static_cast<float>(collisionBox.width), 
									transform.position.y - sin(rad_angle)*static_cast<float>(collisionBox.height)};
				beam.end_point = {transform.position.x + cos(rad_angle)*640.0f, transform.position.y - sin(rad_angle)*360.0f};
				
				beam.collision_rect = {beam.start_point.x, beam.start_point.y, 30.0f,30.0f};
				
				float slope_step = 0.5f;
				
				beam.projectile_speed_x = (beam.end_point.x - beam.start_point.x) / slope_step;
				
				beam.projectile_speed_y = (beam.end_point.y - beam.start_point.y) / slope_step;
							
				beam.energy_beam_attacker_index = energy_attacker.queue_id;
				
				beam.time_active = 0.0f;
			}
			//else if large energy blast launched
			else if(energy_attacker.state == EnergyAttackerState::SEND_LARGE_BLAST)
			{
				//std::cout << "Large energy blast activated!\n";
				
				//take off all energy beams from pool
				queue_energy_pool_available_array[energy_attacker.queue_id] = -1;
				
				energy_attacker.state = EnergyAttackerState::IDLE;
				
				size_t index = 0;
				bool activated = false;
				EnergyAttackSystem::ActivateInLargeEnergyPool(index,activated);
				
				//if no more beams
				if(!activated){return;}
				
				//set small energy beam collision, start, end, and reference to energy beam pool index
				LargeEnergyBlast& blast = large_energy_pool_vector[index];
				
				//std::cout << "blast angle: " << energy_attacker.energy_beam_angle_deg << std::endl;
				float rad_angle = energy_attacker.energy_beam_angle_deg * ( PI / 180.0f);
				
				blast.start_point = {transform.position.x + cos(rad_angle)*static_cast<float>(collisionBox.width), 
									transform.position.y - sin(rad_angle)*static_cast<float>(collisionBox.height)};
				blast.end_point = {transform.position.x + cos(rad_angle)*640.0f, transform.position.y - sin(rad_angle)*360.0f};
				
				blast.collision_rect = {blast.start_point.x, blast.start_point.y, 90.0f,90.0f};
				
				blast.in_beam_struggle = false;
				
				float slope_step = 1.5f;
				
				float speed_x = 0.0f;
				float speed_y = 0.0f;
				
				if(energy_attacker.energy_beam_angle_deg == 0.0f || energy_attacker.energy_beam_angle_deg == 180.0f)
				{
					speed_x = (blast.end_point.x - blast.start_point.x) / slope_step;
					blast.line_end.y = blast.start_point.y;
					blast.line_end.width = 10;
					blast.line_end.height = 90;
					
					if(energy_attacker.energy_beam_angle_deg == 0.0f){blast.face_dir = FaceDirection::EAST;}
					if(energy_attacker.energy_beam_angle_deg == 180.0f){blast.face_dir = FaceDirection::WEST;}
				}
				else if(energy_attacker.energy_beam_angle_deg == 90.0f || energy_attacker.energy_beam_angle_deg == -90.0f)
				{
					speed_y = (blast.end_point.y - blast.start_point.y) / slope_step;
					blast.line_end.x = blast.start_point.x;
					blast.line_end.width = 90;
					blast.line_end.height = 10;
					 
					if(energy_attacker.energy_beam_angle_deg == 90.0f){blast.face_dir = FaceDirection::NORTH;}
					if(energy_attacker.energy_beam_angle_deg == -90.0f){blast.face_dir = FaceDirection::SOUTH;}
				}
				
				blast.projectile_speed_x = speed_x;
				
				blast.projectile_speed_y = speed_y;
							
				blast.energy_beam_attacker_index = energy_attacker.queue_id;
				
				blast.time_active = 0.0f;
				
				//keep energy attacker from moving
				gen_entity_state.actor_state = EntityState::ATTACKING_NO_MOVE;
				blast.entity_state_ptr = &gen_entity_state.actor_state;
			}
			
		}
		else
		{
			
			//for beam struggle
			energy_attacker_energy_button_pressed[energy_attacker.queue_id] = energy_attacker.energy_button_released;
			
			//std::cout << "in beam struggle input: \n\tenergy attacker " << int(energy_attacker.queue_id) 
			//		<< " energy button press: " << energy_attacker_energy_button_pressed[energy_attacker.queue_id] << std::endl;
					
			energy_attacker.energy_button_released = false;
			
			
		}
		
		
		
	}
}

//time limit of 3 seconds for energy projectiles
static const float small_time_limit = 3.0f;

//time limit of 6 seconds for energy blasts
static const float large_time_limit = 6.0f;

void EnergyAttackSystem::HandleEnergyBeamMovement(float& dt)
{
	size_t iterator = 0;
	
	//move each beam in vector
	for( auto& beam : energy_pool_vector)
	{
		//skip if not active
		if(!beam.active){continue;}
		
		//move beam
		beam.collision_rect.x += beam.projectile_speed_x*dt;
		beam.collision_rect.y += beam.projectile_speed_y*dt;
		beam.time_active += dt;
		
		//if time limit passed
		if(beam.time_active > small_time_limit)
		{
			
			//add beam back to energy attacker queue
			queue_energy_pool_available_array[beam.energy_beam_attacker_index] += 1;
			
			//remove beam from vector
			EnergyAttackSystem::DeactivateInSmallEnergyPool(iterator);
			
		}
		
		iterator++;
	}
	
	iterator = 0;
	
	//move each energy blast in vector
	for(auto& blast : large_energy_pool_vector)
	{
		//skip if not active or in beam struggle
		if(!blast.active || blast.in_beam_struggle){iterator++; continue;}
		
		EnergyAttackSystem::MoveBlast(blast,dt);
		
		blast.time_active += dt;
		
		
		//if time limit passed
		if(blast.time_active > large_time_limit)
		{
			//add all beams back to energy attacker queue
			queue_energy_pool_available_array[blast.energy_beam_attacker_index] = MAX_ENERGY_BEAMS_PER_ATTACKER - 1;
			
			//free energy attacker to move again
			*blast.entity_state_ptr = EntityState::NONE;
			
			//remove beam from vector
			EnergyAttackSystem::DeactivateInLargeEnergyPool(iterator);
		}
		
		iterator++;
	}
}

static bool CheckCollisionRectangles(Rectangle& rect_a,Rectangle& rect_b)
{
	//assuming object has width and height of 30 and it is centered
	
	float& objLeftX = rect_a.x;
	float objRightX = rect_a.x + rect_a.width;
	float& objTopY = rect_a.y;
	float objBottomY = rect_a.y + rect_a.height;
	
	float& rectLeftX = rect_b.x;
	float rectRightX = rect_b.x + rect_b.width;
	float& rectTopY = rect_b.y;
	float rectBottomY = rect_b.y + rect_b.height;
	
	//for collision to be true, all conditions must be true. AABB square collsion detection, all
	//The left edge x-position of [A] must be less than the right edge x-position of [B].
    //The right edge x-position of [A] must be greater than the left edge x-position of [B].
    //The top edge y-position of [A] must be less than the bottom edge y-position of [B].
    //The bottom edge y-position of [A] must be greater than the top edge y-position of [B].
    
    if(objBottomY <= rectTopY)
	{
		return false;
	}
	
	if(objTopY >= rectBottomY)
	{
		return false;
	}
    
    if(objRightX <= rectLeftX)
	{
		return false;
	}
	
	if(objLeftX >= rectRightX)
	{
		return false;
	}
	
	return true;
}

void EnergyAttackSystem::HandleEnergyToEnergyCollisions(float& dt)
{
	//check collisions between small energy projectiles
	bool end_check = false;
	size_t left_index = 0;
	size_t right_index = MAX_ENERGY_BEAMS_PER_ATTACKER*MAX_NUM_ATTACKERS - 1;
	
	//while all energy projectiles have not been checked
	while(!end_check)
	{
		//std::cout << "left index: " << left_index << std::endl;
		
		//move on to next left index if left index of energy pool is not active
		if(!energy_pool_vector[left_index].active)
		{
			left_index++;
			
			//break out of loop if left index is at right index
			if(left_index >= right_index){break;}
			 
			continue;
		}
		
		size_t num_elements_check = right_index - left_index;
		for(size_t i = 0; i < num_elements_check; i++)
		{
			
			size_t current_index = right_index - i;
			//skip if index checked is not active in energy pool
			if(!energy_pool_vector[current_index].active){continue;}
			
			//std::cout << "index: " << (current_index) << std::endl;
			
			//if there is a collision between blasts, destroy both
			if(CheckCollisionRectangles(energy_pool_vector[left_index].collision_rect,
										energy_pool_vector[right_index - i].collision_rect))
			{
				auto& beam_one = energy_pool_vector[left_index];
				auto& beam_two = energy_pool_vector[current_index];
				
				//deactivate energy projectiles
				EnergyAttackSystem::DeactivateInSmallEnergyPool(left_index);
				EnergyAttackSystem::DeactivateInSmallEnergyPool(current_index);
				
				//add energy projectile back to queue of energy attackers
				queue_energy_pool_available_array[beam_one.energy_beam_attacker_index] += 1;
				queue_energy_pool_available_array[beam_two.energy_beam_attacker_index] += 1;
				
				
				
			}
		}
		
		left_index++;
		if(left_index == right_index){end_check = true;}
		
	}
	
	//std::cout << "Check is over!\n";		
			
	//check collisions between large energy blasts
	end_check = false;
	left_index = 0;
	right_index = MAX_NUM_ATTACKERS - 1;
	
	//while all energy blasts have not been checked
	while(!end_check)
	{
		//std::cout << "left index: " << left_index << std::endl;
		
		//move on to next left index if left index of energy pool is not active
		if(!large_energy_pool_vector[left_index].active)
		{
			left_index++;
			
			//break out of loop if left index is at right index
			if(left_index >= right_index){break;}
			 
			continue;
		}
		
		size_t num_elements_check = right_index - left_index;
		for(size_t i = 0; i < num_elements_check; i++)
		{
			
			size_t current_index = right_index - i;
			//skip if index checked is not active in energy pool
			if(!large_energy_pool_vector[current_index].active){continue;}
			
			//std::cout << "index: " << (current_index) << std::endl;
			
			//if there is a collision between projectiles, destroy both
			if(CheckCollisionRectangles(large_energy_pool_vector[left_index].collision_rect,
										large_energy_pool_vector[current_index].collision_rect))
			{
				//std::cout << "Beam struggle!\n";
				EnergyAttackSystem::HandleBeamStruggleBetweenTwoBeams(large_energy_pool_vector[left_index],
																	  large_energy_pool_vector[current_index],
																	  dt);
			}
		}
		
		left_index++;
		if(left_index == right_index){end_check = true;}
		
	}
	
	//blast to projectile collisions

	//for all large energy blasts
	for(size_t large_blast_index = 0; large_blast_index < large_energy_pool_vector.size(); large_blast_index++)
	{
		//skip if blast is not active
		if(!large_energy_pool_vector[large_blast_index].active){continue;}
		
		//check against all small energy blasts
		for(size_t i = 0; i < energy_pool_vector.size(); i++)
		{
			//skip if small energy blast is not active
			if(!energy_pool_vector[i].active)
			{
				continue;
			}
			
			//if there is a collision
			if(CheckCollisionRectangles(energy_pool_vector[i].collision_rect,
										large_energy_pool_vector[large_blast_index].collision_rect))
			{
				//eliminate small energy blast
				//add beam back to energy attacker queue
				int8_t& attacker_index = energy_pool_vector[i].energy_beam_attacker_index;
				queue_energy_pool_available_array[attacker_index] += 1;
					
				//remove beam from vector
				EnergyAttackSystem::DeactivateInSmallEnergyPool(i);
			}
		}
	}
}

void EnergyAttackSystem::HandleCollisionWithWorldTiles()
{
	
	//world
	World* world_ptr = &world_one;
	
	//calculate tile that object is on
	
	for( auto& beam : energy_pool_vector)
	{
		if(!beam.active){continue;}
		
		size_t iterator = 0;
		
		float& obj_x = beam.collision_rect.x;
		float& obj_y = beam.collision_rect.y; 
		float& obj_width = beam.collision_rect.width; 
		float& obj_height = beam.collision_rect.height;
		
	
		size_t horiz_index = trunc(obj_x / 30 );
		size_t vert_index = trunc((obj_y + 30) / 30 ) * world_num_tile_horizontal;

		size_t object_tile_index = horiz_index + vert_index; 

		std::array <size_t,9> tiles_around_object;

		if(object_tile_index > world_num_tile_horizontal)
		{
			tiles_around_object[0] = object_tile_index - world_num_tile_horizontal - 1;
			tiles_around_object[1] = object_tile_index - world_num_tile_horizontal;
			tiles_around_object[2] = object_tile_index - world_num_tile_horizontal + 1;
		}
		else
		{
			tiles_around_object[0] = 0;
			tiles_around_object[1] = 0;
			tiles_around_object[2] = 0;
		}

		if(object_tile_index > 0)
		{
			tiles_around_object[3] = object_tile_index - 1;
		}
		else
		{
			tiles_around_object[3] = 0;
		}

		tiles_around_object[4] = object_tile_index;

		tiles_around_object[5] = object_tile_index + 1;

		if(object_tile_index < world_num_tile_horizontal*219)
		{
			tiles_around_object[6] = object_tile_index + world_num_tile_horizontal - 1;
			tiles_around_object[7] = object_tile_index + world_num_tile_horizontal;
			tiles_around_object[8] = object_tile_index + world_num_tile_horizontal + 1;
		}
		else
		{
			tiles_around_object[6] = world_num_tile_horizontal*220 - 3;
			tiles_around_object[7] = world_num_tile_horizontal*220 - 2;
			tiles_around_object[8] = world_num_tile_horizontal*220 - 1;
		}
		
		//for tiles around beam
		for(size_t i = 0; i < tiles_around_object.size(); i++)
		{
			size_t& tile_index = tiles_around_object[i];
			
			//if out of bounds
			if(tile_index > world_num_tile_horizontal*world_num_tile_horizontal - 1)
			{
				//add beam back to energy attacker queue
				queue_energy_pool_available_array[beam.energy_beam_attacker_index] += 1;
				
				//remove beam from vector
				EnergyAttackSystem::DeactivateInSmallEnergyPool(iterator);
				 
				break;
			}
			
			//if regular push back tiles
			if(world_ptr->tiles_vector[tile_index].type == TileType::PUSH_BACK)
			{
				//if object collides with tile
				if(CollisionWithTileDetected(world_ptr->tiles_vector[tile_index].x,world_ptr->tiles_vector[tile_index].y,
								   obj_x, obj_y, obj_width, obj_height) 
					)
				{
					
					//change tile to passable background tile
					world_ptr->tiles_vector[tile_index].type = TileType::BACKGROUND;
					world_ptr->tiles_vector[tile_index].tile_id = 0;
					world_ptr->tiles_vector[tile_index].frame_clip_ptr = &world_ptr->frame_clip_map[0];
					
					//add beam back to energy attacker queue
					queue_energy_pool_available_array[beam.energy_beam_attacker_index] += 1;
					
					//remove beam from vector
					EnergyAttackSystem::DeactivateInSmallEnergyPool(iterator);
				}
			}
			//else if planet destruction tile
			else if(world_ptr->tiles_vector[tile_index].type == TileType::PLANET_DESTRUCTION)
			{
				//if object collides with tile
				if(CollisionWithTileDetected(world_ptr->tiles_vector[tile_index].x,world_ptr->tiles_vector[tile_index].y,
								   obj_x, obj_y, obj_width, obj_height) 
					)
				{
					
					//start planet destruction
					world_ptr->planet_destruction_start = true;
					
					//add beam back to energy attacker queue
					queue_energy_pool_available_array[beam.energy_beam_attacker_index] += 1;
					
					//remove beam from vector
					EnergyAttackSystem::DeactivateInSmallEnergyPool(iterator);
				}
			}

		}		
				
		
		iterator++;
	}
	
	for( auto& blast : large_energy_pool_vector)
	{
		if(!blast.active){continue;}
		
		size_t iterator = 0;
		
		float obj_x = 0.0f;
		float obj_y = 0.0f; 
		float& obj_width = blast.collision_rect.width; 
		float& obj_height = blast.collision_rect.height;
		
		if(blast.face_dir == FaceDirection::WEST)
		{
			obj_x = blast.collision_rect.x;
			obj_y = blast.collision_rect.y;
		}
		else if(blast.face_dir == FaceDirection::EAST)
		{
			obj_x = blast.collision_rect.x + obj_width;
			obj_y = blast.collision_rect.y;
		}
		else if(blast.face_dir == FaceDirection::NORTH)
		{
			obj_x = blast.collision_rect.x;
			obj_y = blast.collision_rect.y;
		}
		else if(blast.face_dir == FaceDirection::SOUTH)
		{
			obj_x = blast.collision_rect.x;
			obj_y = blast.collision_rect.y + obj_height;
		}
		
		size_t horiz_index = trunc((obj_x) / 30 );
		size_t vert_index = trunc((obj_y + 30) / 30 ) * world_num_tile_horizontal;

		size_t object_tile_index = horiz_index + vert_index; 
		
		//std::cout << "blast obj tile index:" << object_tile_index << std::endl;
		
		std::array <size_t,9> tiles_around_object;

		if(object_tile_index > world_num_tile_horizontal)
		{
			tiles_around_object[0] = object_tile_index - world_num_tile_horizontal - 1;
			tiles_around_object[1] = object_tile_index - world_num_tile_horizontal;
			tiles_around_object[2] = object_tile_index - world_num_tile_horizontal + 1;
		}
		else
		{
			tiles_around_object[0] = 0;
			tiles_around_object[1] = 0;
			tiles_around_object[2] = 0;
		}

		if(object_tile_index > 0)
		{
			tiles_around_object[3] = object_tile_index - 1;
		}
		else
		{
			tiles_around_object[3] = 0;
		}

		tiles_around_object[4] = object_tile_index;

		tiles_around_object[5] = object_tile_index + 1;

		if(object_tile_index < world_num_tile_horizontal*219)
		{
			tiles_around_object[6] = object_tile_index + world_num_tile_horizontal - 1;
			tiles_around_object[7] = object_tile_index + world_num_tile_horizontal;
			tiles_around_object[8] = object_tile_index + world_num_tile_horizontal + 1;
		}
		else
		{
			tiles_around_object[6] = world_num_tile_horizontal*world_num_tile_horizontal - 3;
			tiles_around_object[7] = world_num_tile_horizontal*world_num_tile_horizontal - 2;
			tiles_around_object[8] = world_num_tile_horizontal*world_num_tile_horizontal - 1;
		}
		
		//for tiles around blast
		for(size_t i = 0; i < tiles_around_object.size(); i++)
		{
			size_t& tile_index = tiles_around_object[i];
			
			//if out of bounds
			if(tile_index > world_num_tile_horizontal*world_num_tile_horizontal - 1)
			{
				//add beam back to energy attacker queue
				queue_energy_pool_available_array[blast.energy_beam_attacker_index] = MAX_ENERGY_BEAMS_PER_ATTACKER - 1;
				
				//remove blast from vector				
				EnergyAttackSystem::DeactivateInLargeEnergyPool(iterator);
				
				break;
			}
			
			//if regular push back tiles
			if(world_ptr->tiles_vector[tile_index].type == TileType::PUSH_BACK)
			{
				//if object collides with tile
				if(CollisionWithTileDetected(world_ptr->tiles_vector[tile_index].x,world_ptr->tiles_vector[tile_index].y,
								   obj_x, obj_y, obj_width, obj_height) 
					)
				{
					
					//change tile to passable background tile
					world_ptr->tiles_vector[tile_index].type = TileType::BACKGROUND;
					world_ptr->tiles_vector[tile_index].tile_id = 0;
					world_ptr->tiles_vector[tile_index].frame_clip_ptr = &world_ptr->frame_clip_map[0];
					
				}
			}
			//else if planet destruction tile
			else if(world_ptr->tiles_vector[tile_index].type == TileType::PLANET_DESTRUCTION)
			{
				//if object collides with tile
				if(CollisionWithTileDetected(world_ptr->tiles_vector[tile_index].x,world_ptr->tiles_vector[tile_index].y,
								   obj_x, obj_y, obj_width, obj_height) 
					)
				{
					
					//start planet destruction
					world_ptr->planet_destruction_start = true;
					
					//add beam back to energy attacker queue
					queue_energy_pool_available_array[blast.energy_beam_attacker_index] = MAX_ENERGY_BEAMS_PER_ATTACKER - 1;
					
					//remove beam from vector
					EnergyAttackSystem::DeactivateInLargeEnergyPool(iterator);
					
				}
			}

		}		
				
		
		iterator++;
	}
}

static bool CollisionWithRectangleDetected(Rectangle& rect,
						   float& obj_x, float& obj_y, std::uint32_t& obj_width, std::uint32_t& obj_height)
{
	//assuming object has width and height of 30 and it is centered
	
	float objLeftX = obj_x;
	float objRightX = obj_x + obj_width;
	float objTopY = obj_y;
	float objBottomY = obj_y + obj_height;
	
	std::uint32_t rectLeftX = rect.x;
	std::uint32_t rectRightX = rect.x + rect.width;
	std::uint32_t rectTopY = rect.y;
	std::uint32_t rectBottomY = rect.y + rect.height;
	
	//for collision to be true, all conditions must be true. AABB square collsion detection, all
	//The left edge x-position of [A] must be less than the right edge x-position of [B].
    //The right edge x-position of [A] must be greater than the left edge x-position of [B].
    //The top edge y-position of [A] must be less than the bottom edge y-position of [B].
    //The bottom edge y-position of [A] must be greater than the top edge y-position of [B].
    
    if(objBottomY <= rectTopY)
	{
		return false;
	}
	
	if(objTopY >= rectBottomY)
	{
		return false;
	}
    
    if(objRightX <= rectLeftX)
	{
		return false;
	}
	
	if(objLeftX >= rectRightX)
	{
		return false;
	}
	
	return true;
}

void EnergyAttackSystem::HandleCollisionWithGeneralActors()
{
	for(auto entity : mEntities)
	{
		auto& transform = gCoordinator.GetComponent<Transform2D>(entity);
		auto& collisionBox = gCoordinator.GetComponent<CollisionBox>(entity);
		auto& gen_entity_state = gCoordinator.GetComponent<GeneralEnityState>(entity);
		
		size_t iterator = 0;
		//for every small beam
		//check if there is a collision
		for( auto& beam : energy_pool_vector)
		{
			if(!beam.active){continue;}
			
			//if beam collision rectangle collides with a general actor i.e. player,enemy,object
			if( CollisionWithRectangleDetected(beam.collision_rect,
												transform.position.x, transform.position.y, 
												collisionBox.width, collisionBox.height))	
			{
				//decrease health
				gen_entity_state.health -= 4;
				
				//put in general actor in state of taking damage
				gen_entity_state.taking_damage = true;
				gen_entity_state.actor_state = EntityState::HURTING_KNOCKBACK;
				
				//move back player in opposite direction of beam
				float sign_x = 1.0f; 
				if(beam.projectile_speed_x < -10.0f ){sign_x = -1.0f;}
				
				float sign_y = 1.0f;
				if(beam.projectile_speed_y < -10.0f){sign_y = -1.0f;}
				
				float tiles_knock = 1.0f;
				
				gen_entity_state.victim_knockback_amt.x = sign_x*tiles_knock;
				gen_entity_state.victim_knockback_amt.y = sign_y*tiles_knock;
				
				//add beam back to energy attacker queue
				queue_energy_pool_available_array[beam.energy_beam_attacker_index] += 1;
					
				//remove beam from vector
				EnergyAttackSystem::DeactivateInSmallEnergyPool(iterator);
			}
			
			iterator++;
		}
		
		//for large blasts
		//check if there is a collision
		for( auto& blast : large_energy_pool_vector)
		{
			if(!blast.active){continue;}
			
			//if beam collision rectangle collides with a general actor i.e. player,enemy,object
			if( CollisionWithRectangleDetected(blast.collision_rect,
												transform.position.x, transform.position.y, 
												collisionBox.width, collisionBox.height))	
			{
				//if collision is not with energy attacker who made blast
				if(blast.entity_state_ptr != &gen_entity_state.actor_state)
				{
					//decrease health
					gen_entity_state.health -= 15;
					
					//put in general actor in state of taking damage
					gen_entity_state.taking_damage = true;
					gen_entity_state.actor_state = EntityState::HURTING_KNOCKBACK;
					
					//do not knock back vitcim
					
					gen_entity_state.victim_knockback_amt.x = 0.0f;
					gen_entity_state.victim_knockback_amt.y = 0.0f;
					
					//add beam back to energy attacker queue
					queue_energy_pool_available_array[blast.energy_beam_attacker_index] = MAX_ENERGY_BEAMS_PER_ATTACKER - 1;
						
					//remove beam from vector
					blast.active = false;
					blast.in_beam_struggle = false;
					
					
				}
				
			}
			
		}
	}
	
}

void EnergyAttackSystem::RenderEnergyBeams_VersusMode(CustomCamera* camera_ptr)
{
	if(camera_ptr)
	{			
		
		Rectangle* camera_rect_ptr = camera_ptr->GetCameraRectPointer();
		
		//for small energy beams
		for( auto& beam : energy_pool_vector)
		{
			if(!beam.active){continue;}
			
			if(beam.collision_rect.x > camera_rect_ptr->x && beam.collision_rect.y > camera_rect_ptr->y
				&& beam.collision_rect.x < camera_rect_ptr->x + camera_rect_ptr->width
				&& beam.collision_rect.y < camera_rect_ptr->y + camera_rect_ptr->height)
			{
				DrawRectangle(beam.collision_rect.x - camera_rect_ptr->x, 
					beam.collision_rect.y - camera_rect_ptr->y, 
						beam.collision_rect.width, beam.collision_rect.height, 
						RED);
			}
						
			
		}
						
		
		for(auto& blast: large_energy_pool_vector)
		{
			if(!blast.active){continue;}
			
			//energy blast is big so no bounds check.
			DrawRectangle(blast.collision_rect.x - camera_rect_ptr->x, 
					blast.collision_rect.y - camera_rect_ptr->y, 
					blast.collision_rect.width, blast.collision_rect.height, 
					RED);	
			
			//draw the line ending of blast
			DrawRectangle(blast.line_end.x - camera_rect_ptr->x, 
					blast.line_end.y - camera_rect_ptr->y, 
					blast.line_end.width, blast.line_end.height, 
					WHITE);
			
		}
		
		//render charging
		for(auto entity : mEntities)
		{
			auto& energy_attacker = gCoordinator.GetComponent<EnergyAttacker>(entity);
			
			if(energy_attacker.state == EnergyAttackerState::CHARGING ||
				energy_attacker.state == EnergyAttackerState::READY_TO_SEND_LARGE_BLAST)
			{
				
				auto& transform = gCoordinator.GetComponent<Transform2D>(entity);
				auto& collisionBox = gCoordinator.GetComponent<CollisionBox>(entity);
				
				float rad_angle = energy_attacker.energy_beam_angle_deg * ( PI / 180.0f);
			
				Rectangle charge_rect = {transform.position.x + cos(rad_angle)*static_cast<float>(collisionBox.width), 
											transform.position.y - sin(rad_angle)*static_cast<float>(collisionBox.height), 
											30.0f,30.0f};
				
				
				if(energy_attacker.state == EnergyAttackerState::READY_TO_SEND_LARGE_BLAST)
				{
					charge_rect.width = 90.0f;
					charge_rect.height = 90.0f;
				}
				
				DrawRectangle(charge_rect.x - camera_rect_ptr->x, 
					charge_rect.y - camera_rect_ptr->y, 
					charge_rect.width, charge_rect.height, 
					RED);
			}
		}
					
	}
}

static bool beam_struggle_sound = false;

void EnergyAttackSystem::SoundEnergyBeams_VersusMode()
{
	
	
	//sounds specific to energy attacker state 
	for(auto entity : mEntities)
	{
		auto& energy_attacker = gCoordinator.GetComponent<EnergyAttacker>(entity);
		
		//if energy attacker is charging
		if(energy_attacker.state == EnergyAttackerState::CHARGING)
		{
			//play energy charging sound
		}
		//if energy attacker sent a small energy projectile
		else if(energy_attacker.energy_button_released && 
				energy_attacker.state != EnergyAttackerState::READY_TO_SEND_LARGE_BLAST)
		{
			//play small energy projectile sound
			PlayGeneralSound(GeneralSoundID::SMALL_ENERGY_PROJECTILE);
		}
		
	}
	
	if(beam_struggle_sound)
	{
		PlayGeneralSound(GeneralSoundID::BEAM_STRUGGLE_SAMPLE);
		beam_struggle_sound = false;
	}
	
}

void EnergyAttackSystem::RenderEnergyBeams_FreeplayMode(CameraManager& camera_manager_ptr)
{
	
	for(size_t i = 0; i < camera_manager_ptr.screens.size(); i++)
	{
		if(camera_manager_ptr.screens[i].in_active_use)
		{
			if(camera_manager_ptr.screens[i].camera_ptr)
			{
				Rectangle* camera_rect_ptr = camera_manager_ptr.screens[i].camera_ptr->GetCameraRectPointer();
				
				if(!camera_rect_ptr){continue;}
				
				//for small energy beams
				for( auto& beam : energy_pool_vector)
				{
					if(!beam.active){continue;}
					
					if(beam.collision_rect.x > camera_rect_ptr->x && beam.collision_rect.y > camera_rect_ptr->y
						&& beam.collision_rect.x < camera_rect_ptr->x + camera_rect_ptr->width
						&& beam.collision_rect.y < camera_rect_ptr->y + camera_rect_ptr->height)
					{
						DrawRectangle(beam.collision_rect.x - camera_rect_ptr->x, 
							beam.collision_rect.y - camera_rect_ptr->y, 
								beam.collision_rect.width, beam.collision_rect.height, 
								RED);
					}
								
					
				}
								
				
				for(auto& blast: large_energy_pool_vector)
				{
					if(!blast.active){continue;}
					
					//energy blast is big so no bounds check.
					DrawRectangle(blast.collision_rect.x - camera_rect_ptr->x, 
							blast.collision_rect.y - camera_rect_ptr->y, 
							blast.collision_rect.width, blast.collision_rect.height, 
							RED);	
					
					//draw the line ending of blast
					DrawRectangle(blast.line_end.x - camera_rect_ptr->x, 
							blast.line_end.y - camera_rect_ptr->y, 
							blast.line_end.width, blast.line_end.height, 
							WHITE);
					
				}
				
				//render charging
				for(auto entity : mEntities)
				{
					auto& energy_attacker = gCoordinator.GetComponent<EnergyAttacker>(entity);
					
					if(energy_attacker.state == EnergyAttackerState::CHARGING ||
						energy_attacker.state == EnergyAttackerState::READY_TO_SEND_LARGE_BLAST)
					{
						
						auto& transform = gCoordinator.GetComponent<Transform2D>(entity);
						auto& collisionBox = gCoordinator.GetComponent<CollisionBox>(entity);
						
						float rad_angle = energy_attacker.energy_beam_angle_deg * ( PI / 180.0f);
					
						Rectangle charge_rect = {transform.position.x + cos(rad_angle)*static_cast<float>(collisionBox.width), 
													transform.position.y - sin(rad_angle)*static_cast<float>(collisionBox.height), 
													30.0f,30.0f};
						
						
						if(energy_attacker.state == EnergyAttackerState::READY_TO_SEND_LARGE_BLAST)
						{
							charge_rect.width = 90.0f;
							charge_rect.height = 90.0f;
						}
						
						DrawRectangle(charge_rect.x - camera_rect_ptr->x, 
							charge_rect.y - camera_rect_ptr->y, 
							charge_rect.width, charge_rect.height, 
							RED);
					}
				}
				
			}
			else
			{
				std::cout << "Uninitialized camera for active screen!\n";
			}
		}
		
	}
			
}

void EnergyAttackSystem::ActivateInSmallEnergyPool(size_t& index, bool& activated)
{
	size_t search_index = 0;
	
	//look for available energy in pool
	for(search_index = 0; search_index < energy_pool_vector.size(); search_index++)
	{
		if(!energy_pool_vector[search_index].active)
		{
			energy_pool_vector[search_index].active = true;
			activated = true;
			index = search_index;
			return;
		} 
		
	}
	
	activated = false;
	
}

void EnergyAttackSystem::ActivateInLargeEnergyPool(size_t& index, bool& activated)
{
	size_t search_index = 0;
	
	//look for available energy in pool
	for(search_index = 0; search_index < large_energy_pool_vector.size(); search_index++)
	{
		if(!large_energy_pool_vector[search_index].active)
		{
			large_energy_pool_vector[search_index].active = true;
			activated = true;
			index = search_index;
			return;
		} 
		
	}
	
	activated = false;
	
}

void EnergyAttackSystem::DeactivateInSmallEnergyPool(size_t& iterator)
{
	energy_pool_vector[iterator].active = false;
}

void EnergyAttackSystem::DeactivateInLargeEnergyPool(size_t& iterator)
{	
	large_energy_pool_vector[iterator].active = false;
	large_energy_pool_vector[iterator].in_beam_struggle = false;
}

static const float beam_struggle_lose_dimension = 20.0f;

void EnergyAttackSystem::HandleBeamStruggleBetweenTwoBeams(LargeEnergyBlast& blast_one, LargeEnergyBlast& blast_two, float& dt)
{
	
	//if blasts not in opposite directions, but perpendicular
	if(!(blast_one.face_dir == FaceDirection::EAST && blast_two.face_dir == FaceDirection::WEST) &&
	   !(blast_two.face_dir == FaceDirection::EAST && blast_one.face_dir == FaceDirection::WEST) &&
	   !(blast_one.face_dir == FaceDirection::SOUTH && blast_two.face_dir == FaceDirection::NORTH) &&
	   !(blast_one.face_dir == FaceDirection::NORTH && blast_two.face_dir == FaceDirection::SOUTH) )
	{
		//cancel both blasts
		blast_one.active = false;
		blast_two.active = false;
		
		blast_one.in_beam_struggle = false;
		blast_two.in_beam_struggle = false;
		
		queue_energy_pool_available_array[blast_one.energy_beam_attacker_index] = MAX_ENERGY_BEAMS_PER_ATTACKER - 1;
		queue_energy_pool_available_array[blast_two.energy_beam_attacker_index] = MAX_ENERGY_BEAMS_PER_ATTACKER - 1;
		
		*blast_one.entity_state_ptr = EntityState::NONE;
		*blast_two.entity_state_ptr = EntityState::NONE;
		
		return;
	}
	
	
	//put both blasts in beam struggle state
	blast_one.in_beam_struggle = true;
	blast_two.in_beam_struggle = true;
	
	
	bool& blast_one_energy_press = energy_attacker_energy_button_pressed[blast_one.energy_beam_attacker_index];
	//std::cout << "blast one energy attacker " << int(blast_one.energy_beam_attacker_index) 
	//	<< " energy button press: " << blast_one_energy_press << std::endl;
	bool& blast_two_energy_press = energy_attacker_energy_button_pressed[blast_two.energy_beam_attacker_index];
	//std::cout << "blast two energy attacker " << int(blast_two.energy_beam_attacker_index) 
	//	<< " energy button press: " << blast_two_energy_press << std::endl;
		
	LargeEnergyBlast* grow_blast = nullptr;
	LargeEnergyBlast* weaken_blast = nullptr;
	
	//check if energy attackers of blasts have pressed energy button
	//if both players pressed on same frame
	if(blast_one_energy_press && blast_two_energy_press)
	{
		beam_struggle_sound = false;
		//do nothing
		return;
	}
	//if energy attackers both did not press
	else if(!blast_one_energy_press && !blast_two_energy_press)
	{
		beam_struggle_sound = false;
		//do nothing
		return;
	}
	//else if blast one player pressed and the other did not
	else if(blast_one_energy_press && !blast_two_energy_press)
	{
		//grow blast one
		grow_blast = &blast_one;
		weaken_blast = &blast_two;
		beam_struggle_sound = true;
		//std::cout << "blast one attack grows.\n";
	}
	//else if blast two player pressed and the other did not
	else if(!blast_one_energy_press && blast_two_energy_press)
	{
		//grow blast two
		grow_blast = &blast_two;
		weaken_blast = &blast_one;
		beam_struggle_sound = true;
		//std::cout << "blast two attack grows.\n";
	}
	
	
	
	//grow the blast that has gotten more energy input if only one energy attacker pressed button
	EnergyAttackSystem::MoveBlastBeamStruggle(*grow_blast,dt);
	//weaken the blast that has gotten no energy input  from energy attacker
	EnergyAttackSystem::MoveBlastBackBeamStruggle(*weaken_blast,dt);
	
	//printf("After move.\n");
	
	//printf("blast one collision rect: %f, %f, %f, %f: \n",blast_one.collision_rect.x,blast_one.collision_rect.y,
	//		blast_one.collision_rect.width,blast_one.collision_rect.height);
			
	//printf("blast two collision rect: %f, %f, %f, %f: \n",blast_two.collision_rect.x,blast_two.collision_rect.y,
	//		blast_two.collision_rect.width,blast_two.collision_rect.height); 
	
	//if blast two lost
	if(blast_two.collision_rect.width <= beam_struggle_lose_dimension || blast_two.collision_rect.height <= beam_struggle_lose_dimension)
	{
		//deactivate blast two
		blast_two.active = false;
		
		blast_two.in_beam_struggle = false;
		blast_one.in_beam_struggle = false;
		
		//allow blast one energy attacker to move again and have energy again
		*blast_one.entity_state_ptr = EntityState::NONE;
		queue_energy_pool_available_array[blast_one.energy_beam_attacker_index] = MAX_ENERGY_BEAMS_PER_ATTACKER - 1;
		
		queue_energy_pool_available_array[blast_two.energy_beam_attacker_index] = MAX_ENERGY_BEAMS_PER_ATTACKER - 1;
	}
	//else if blast one lost
	else if(blast_one.collision_rect.width <= beam_struggle_lose_dimension || blast_one.collision_rect.height <= beam_struggle_lose_dimension)
	{
		//deactivate blast one
		blast_one.active = false;
		
		blast_one.in_beam_struggle = false;
		blast_two.in_beam_struggle = false;
		
		//allow blast two energy attacker to move again and have energy again
		*blast_two.entity_state_ptr = EntityState::NONE;
		queue_energy_pool_available_array[blast_two.energy_beam_attacker_index] = MAX_ENERGY_BEAMS_PER_ATTACKER - 1;
		
		queue_energy_pool_available_array[blast_one.energy_beam_attacker_index] = MAX_ENERGY_BEAMS_PER_ATTACKER - 1;
	}
	
}

void EnergyAttackSystem::MoveBlast(LargeEnergyBlast& blast, float& dt)
{
	//if moving left
	if(blast.face_dir == FaceDirection::WEST )
	{
		//skip if blast collision is out of bounds
		if(blast.collision_rect.x > 0.0f)
		{
			//move left
			blast.collision_rect.x += blast.projectile_speed_x*dt;
			//keep same distance between right end and player
			blast.collision_rect.width += blast.start_point.x - (blast.collision_rect.x + blast.collision_rect.width);
			//move line end of beam
			blast.line_end.x = blast.collision_rect.x;
			
		}
		
		
	}
	//moving right
	else if(blast.face_dir == FaceDirection::EAST)
	{
		//if width is nout out of bounds
		if(blast.collision_rect.width < 0.5f*world_num_tile_horizontal*30.0f)
		{
			//make beam increase in height and width
			blast.collision_rect.width += blast.projectile_speed_x*dt;
			//move line end of beam
			blast.line_end.x = blast.collision_rect.x + blast.collision_rect.width;
		}
		
	}
	//if moving up
	else if(blast.face_dir == FaceDirection::NORTH)
	{
		//if blast collision is not out of bounds
		if(blast.collision_rect.y > 0.0f)
		{
			//move up 
			blast.collision_rect.y += blast.projectile_speed_y*dt;
			//keep same distance between bottom end and player
			blast.collision_rect.height += blast.start_point.y - (blast.collision_rect.y + blast.collision_rect.height - 60);
			//move line end of beam
			blast.line_end.y = blast.collision_rect.y;
		}
		
	}
	//else if moving down
	else if(blast.face_dir == FaceDirection::SOUTH)
	{
		//if height is out out of bounds
		if(blast.collision_rect.height < 0.5f*world_num_tile_horizontal*30.0f)
		{
			blast.collision_rect.height += blast.projectile_speed_y*dt;
			//move line end of beam
			blast.line_end.y = blast.collision_rect.y + blast.collision_rect.height;
		}
		
	}
}

static const float beam_struggle_speed = 500.0f; //pixels per frame

void EnergyAttackSystem::MoveBlastBeamStruggle(LargeEnergyBlast& blast, float& dt)
{
	//if moving left
	if(blast.face_dir == FaceDirection::WEST )
	{
		//skip if blast collision is out of bounds
		if(blast.collision_rect.x > 0.0f)
		{
			//move left
			blast.collision_rect.x -= beam_struggle_speed*dt;
			//keep same distance between right end and player
			blast.collision_rect.width += blast.start_point.x - (blast.collision_rect.x + blast.collision_rect.width);
			//move line end of beam
			blast.line_end.x = blast.collision_rect.x;
			
		}
		
		
	}
	//moving right
	else if(blast.face_dir == FaceDirection::EAST)
	{
		//if width is nout out of bounds
		if(blast.collision_rect.width < 0.5f*world_num_tile_horizontal*30.0f)
		{
			//make beam increase in height and width
			blast.collision_rect.width += beam_struggle_speed*dt;
			//move line end of beam
			blast.line_end.x = blast.collision_rect.x + blast.collision_rect.width;
		}
		
	}
	//if moving up
	else if(blast.face_dir == FaceDirection::NORTH)
	{
		//if blast collision is not out of bounds
		if(blast.collision_rect.y > 0.0f)
		{
			//move up 
			blast.collision_rect.y -= beam_struggle_speed*dt;
			//keep same distance between bottom end and player
			blast.collision_rect.height += beam_struggle_speed*dt;
			//move line end of beam
			blast.line_end.y = blast.collision_rect.y;
		}
		
	}
	//else if moving down
	else if(blast.face_dir == FaceDirection::SOUTH)
	{
		//if height is out of bounds
		if(blast.collision_rect.height < 0.5f*world_num_tile_horizontal*30.0f)
		{
			blast.collision_rect.height += beam_struggle_speed*dt;
			//move line end of beam
			blast.line_end.y = blast.collision_rect.y + blast.collision_rect.height;
		}
		
	}
}

void EnergyAttackSystem::MoveBlastBackBeamStruggle(LargeEnergyBlast& blast, float& dt)
{
	//if moving left
	if(blast.face_dir == FaceDirection::WEST )
	{
		//skip if blast collision is out of bounds
		if(blast.collision_rect.x > 0.0f)
		{
			//move right
			blast.collision_rect.x += beam_struggle_speed*dt;
			//keep same distance between right end and player
			//blast.collision_rect.width -= blast.start_point.x - (blast.collision_rect.x + blast.collision_rect.width);
			blast.collision_rect.width -= beam_struggle_speed*dt;
			//move line end of beam
			blast.line_end.x = blast.collision_rect.x;
			
		}
		
		
	}
	//moving right
	else if(blast.face_dir == FaceDirection::EAST)
	{
		//if width is not out out of bounds
		if(blast.collision_rect.width < 0.5f*world_num_tile_horizontal*30.0f)
		{
			//make beam decrease in width
			blast.collision_rect.width -= beam_struggle_speed*dt;
			//move line end of beam
			blast.line_end.x = blast.collision_rect.x + blast.collision_rect.width;
		}
		
	}
	//if moving up
	else if(blast.face_dir == FaceDirection::NORTH)
	{
		//if blast collision is not out of bounds
		if(blast.collision_rect.y > 0.0f)
		{
			//move down 
			blast.collision_rect.y += beam_struggle_speed*dt;
			//keep same distance between bottom end and player
			blast.collision_rect.height -= beam_struggle_speed*dt;
			//move line end of beam
			blast.line_end.y = blast.collision_rect.y;
		}
		
	}
	//else if moving down
	else if(blast.face_dir == FaceDirection::SOUTH)
	{
		//if height is out out of bounds
		if(blast.collision_rect.height < 0.5f*world_num_tile_horizontal*30.0f)
		{
			blast.collision_rect.height -= beam_struggle_speed*dt;
			//move line end of beam
			blast.line_end.y = blast.collision_rect.y + blast.collision_rect.height;
		}
		
	}
}


void EnergyAttackSystem::DeactivateAllEnergyAttacks()
{
	//for small energy beams
	for( auto& beam : energy_pool_vector)
	{
		beam.active = false;
	}
	
	//for large energy blasts
	for( auto& blast : large_energy_pool_vector)
	{
		blast.active = false;
		
	}
}
