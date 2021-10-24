
#include "EnergyAttackSystem.h"

#include "core/system.h"

#include "core/coordinator.h"

#include <iostream>

#include <cmath>

#include "misc/level_maps.h"

extern Coordinator gCoordinator;

void EnergyAttackSystem::Init()
{
	//initialize queue for all attackers
	for(size_t i = 0; i < MAX_NUM_ATTACKERS; i++)
	{
		queue_energy_pool_available_array[i] = MAX_ENERGY_BEAMS_PER_ATTACKER;
		
	}
	
	energy_pool_vector.reserve(MAX_ENERGY_BEAMS_PER_ATTACKER*MAX_NUM_ATTACKERS);
	
	large_energy_pool_vector.reserve(MAX_NUM_ATTACKERS);
	
}

void EnergyAttackSystem::HandleEnergyBeamActivation()
{
	for (auto const& entity : mEntities)
	{
		auto& energy_attacker = gCoordinator.GetComponent<EnergyAttacker>(entity);
		auto& transform = gCoordinator.GetComponent<Transform2D>(entity);
		auto& collisionBox = gCoordinator.GetComponent<CollisionBox>(entity);
		auto& gen_entity_state = gCoordinator.GetComponent<GeneralEnityState>(entity);
		
		//if energy beams available from queue and energy beam requested
		if(queue_energy_pool_available_array[energy_attacker.queue_id] != -1 
			)
		{
			//if sending energy beam
			if(energy_attacker.send_energy_beam)
			{
				//take off 1 energy beam from available pool
				queue_energy_pool_available_array[energy_attacker.queue_id] -= 1;
				
				energy_attacker.send_energy_beam = false;
				
				//add to vector of energy beams on screen
				energy_pool_vector.emplace_back(SmallEnergyBeam());
				
				//set small energy beam collision, start, end, and reference to energy beam pool index
				SmallEnergyBeam& beam = energy_pool_vector.back();
				
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
			//else if large energy blast
			else if(energy_attacker.energy_blast)
			{
				std::cout << "Large energy blast activated!\n";
				
				//take off all energy beams from pool
				queue_energy_pool_available_array[energy_attacker.queue_id] = -1;
				
				energy_attacker.energy_blast = false;
				
				//add to vector of energy beams on screen
				large_energy_pool_vector.emplace_back(LargeEnergyBlast());
				
				//set small energy beam collision, start, end, and reference to energy beam pool index
				LargeEnergyBlast& blast = large_energy_pool_vector.back();
				
				float rad_angle = energy_attacker.energy_beam_angle_deg * ( PI / 180.0f);
				blast.start_point = {transform.position.x + cos(rad_angle)*static_cast<float>(collisionBox.width), 
									transform.position.y - sin(rad_angle)*static_cast<float>(collisionBox.height)};
				blast.end_point = {transform.position.x + cos(rad_angle)*640.0f, transform.position.y - sin(rad_angle)*360.0f};
				
				blast.collision_rect = {blast.start_point.x, blast.start_point.y, 90.0f,90.0f};
				
				float slope_step = 1.5f;
				
				blast.projectile_speed_x = (blast.end_point.x - blast.start_point.x) / slope_step;
				
				blast.projectile_speed_y = (blast.end_point.y - blast.start_point.y) / slope_step;
							
				blast.energy_beam_attacker_index = energy_attacker.queue_id;
				
				blast.time_active = 0.0f;
				
				//keep energy attacker from moving
				gen_entity_state.actor_state = EntityState::ATTACKING_NO_MOVE;
				blast.entity_state_ptr = &gen_entity_state.actor_state;
			}
			
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
			std::swap(energy_pool_vector[iterator],energy_pool_vector.back());
			energy_pool_vector.pop_back();
			
		}
		
		iterator++;
	}
	
	iterator = 0;
	
	//move each energy blast in vector
	for(auto& blast : large_energy_pool_vector)
	{
		
		
		//if moving left
		if(blast.projectile_speed_x < -2.0f )
		{
			//skip if blast collision is out of bounds
			if(blast.collision_rect.x > 0.0f)
			{
				//move left
				blast.collision_rect.x += blast.projectile_speed_x*dt;
				//keep same distance between right end and player
				blast.collision_rect.width += blast.start_point.x - (blast.collision_rect.x + blast.collision_rect.width);
			}
			
			
		}
		else
		{
			//make beam increase in height and width
			blast.collision_rect.width += blast.projectile_speed_x*dt;
		}
		
		
		
		//if moving up
		if(blast.projectile_speed_y < -2.0f)
		{
			//if blast collision is not out of bounds
			if(blast.collision_rect.y > 0.0f)
			{
				//move up 
				blast.collision_rect.y += blast.projectile_speed_y*dt;
				//keep same distance between bottom end and player
				blast.collision_rect.height += blast.start_point.y - (blast.collision_rect.y + blast.collision_rect.height - 60);
			}
			
		}
		else
		{
			blast.collision_rect.height += blast.projectile_speed_y*dt;
		}
		
		blast.time_active += dt;
		
		
		//if time limit passed
		if(blast.time_active > large_time_limit)
		{
			//add all beams back to energy attacker queue
			queue_energy_pool_available_array[blast.energy_beam_attacker_index] = 4;
			
			//free energy attacker to move again
			*blast.entity_state_ptr = EntityState::NONE;
			
			//remove beam from vector
			std::swap(large_energy_pool_vector[iterator],large_energy_pool_vector.back());
			large_energy_pool_vector.pop_back();
		}
		
		iterator++;
	}
}

void EnergyAttackSystem::HandleCollisionWithWorldTiles()
{
	
	//world
	World* world_ptr = &world_one;
	
	//calculate tile that object is on
	
	for( auto& beam : energy_pool_vector)
	{
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
				std::swap(energy_pool_vector[iterator],energy_pool_vector.back());
				energy_pool_vector.pop_back();
				 
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
					std::swap(energy_pool_vector[iterator],energy_pool_vector.back());
					energy_pool_vector.pop_back();
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
					std::swap(energy_pool_vector[iterator],energy_pool_vector.back());
					energy_pool_vector.pop_back();
				}
			}

		}		
				
		
		iterator++;
	}
	
	for( auto& blast : large_energy_pool_vector)
	{
		size_t iterator = 0;
		
		float& obj_x = blast.collision_rect.x;
		float& obj_y = blast.collision_rect.y; 
		float& obj_width = blast.collision_rect.width; 
		float& obj_height = blast.collision_rect.height;
		
	
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
		
		//for tiles around blast
		for(size_t i = 0; i < tiles_around_object.size(); i++)
		{
			size_t& tile_index = tiles_around_object[i];
			
			//if out of bounds
			if(tile_index > world_num_tile_horizontal*world_num_tile_horizontal - 1)
			{
				//add beam back to energy attacker queue
				queue_energy_pool_available_array[blast.energy_beam_attacker_index] = 4;
				
				//remove blast from vector
				std::swap(large_energy_pool_vector[iterator],large_energy_pool_vector.back());
				large_energy_pool_vector.pop_back();
				 
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
					queue_energy_pool_available_array[blast.energy_beam_attacker_index] = 4;
					
					//remove beam from vector
					std::swap(large_energy_pool_vector[iterator],large_energy_pool_vector.back());
					large_energy_pool_vector.pop_back();
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
				std::swap(energy_pool_vector[iterator],energy_pool_vector.back());
				energy_pool_vector.pop_back();
			}
			
			iterator++;
		}
		
		iterator = 0;
		//for large blasts
		//check if there is a collision
		for( auto & blast : large_energy_pool_vector)
		{
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
					queue_energy_pool_available_array[blast.energy_beam_attacker_index] = 4;
						
					//remove beam from vector
					std::swap(large_energy_pool_vector[iterator],large_energy_pool_vector.back());
					large_energy_pool_vector.pop_back();
				}
				
			}
			
			iterator++;
		}
	}
	
}


void EnergyAttackSystem::RenderEnergyBeams_FreeplayMode(CameraManager* camera_manager_ptr)
{
	
	for(size_t i = 0; i < camera_manager_ptr->screens.size(); i++)
	{
		if(camera_manager_ptr->screens[i].in_active_use)
		{
			if(camera_manager_ptr->screens[i].camera_ptr)
			{
				Rectangle* camera_rect_ptr = camera_manager_ptr->screens[i].camera_ptr->GetCameraRectPointer();
				
				if(!camera_rect_ptr){continue;}
				
				//for small energy beams
				for( auto& beam : energy_pool_vector)
				{
					if(beam.collision_rect.x < camera_rect_ptr->x || beam.collision_rect.y < camera_rect_ptr->y)
					{
						continue;
					}
								
					DrawRectangle(beam.collision_rect.x - camera_rect_ptr->x, 
							beam.collision_rect.y - camera_rect_ptr->y, 
								beam.collision_rect.width, beam.collision_rect.height, 
								RED);
				}
				
				//for large energy blast
				for(auto& blast: large_energy_pool_vector)
				{
					if(blast.start_point.x < camera_rect_ptr->x || blast.start_point.y < camera_rect_ptr->y)
					{
						continue;
					}	
					
					DrawRectangle(blast.collision_rect.x - camera_rect_ptr->x, 
							blast.collision_rect.y - camera_rect_ptr->y, 
								blast.collision_rect.width, blast.collision_rect.height, 
								RED);
				}
				
			}
			else
			{
				std::cout << "Uninitialized camera for active screen!\n";
			}
		}
		
	}
			
}
