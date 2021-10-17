#include "CraftingSystem.h"

#include "core/system.h"

#include "core/coordinator.h"

#include <iostream>

#include <cmath>

extern Coordinator gCoordinator;

void CraftingSystem::Init()
{
	
}

void CraftingSystem::HandleCrafting()
{
	//world
	World* world_ptr = &world_one;
	
	CraftingSystem::HandleTilePlacement(world_ptr);
}

void CraftingSystem::HandleTilePlacement(World* world_ptr)
{
	for (auto const& entity : mEntities)
	{
		
		auto& player = gCoordinator.GetComponent<Player>(entity);
		
		//if player craft button pressed
		if(player.craftButtonPressed)
		{
			
			//get direction player is facing
			auto& animation = gCoordinator.GetComponent<Animation>(entity);
			auto& transform = gCoordinator.GetComponent<Transform2D>(entity);
			
			//get tile near player in direction facing
			
			size_t num_tile_horizontal = 220;
			
			size_t horiz_index = trunc(transform.position.x / 30 );
			size_t vert_index = trunc((transform.position.y + 30) / 30 ) * num_tile_horizontal;

			size_t player_tile = horiz_index + vert_index;
			
			size_t tile_to_change = 0;
			
			//To do: Bounds checking at left and right border.
			//Also top and bottom border
			
			switch(animation.face_dir)
			{
				case FaceDirection::WEST:
				{
					tile_to_change = player_tile - 1;
					break;
				}
				case FaceDirection::EAST:
				{ 
					tile_to_change = player_tile + 1; 
					break;
				}
				default:
				{
					tile_to_change = player_tile;
					break;
				}
			}
						
			//change tile type, frame, tile id
			world_ptr->tiles_vector[tile_to_change].type = TileType::PUSH_BACK;
			world_ptr->tiles_vector[tile_to_change].tile_id = 2;
			world_ptr->tiles_vector[tile_to_change].frame_clip_ptr = &world_ptr->frame_clip_map[2];
			
			player.craftButtonPressed = false;
		}
		
	}
	
}
