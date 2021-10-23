#include "level_maps.h"

bool CollisionWithTileDetected(float tile_x,float tile_y,
						   float& obj_x, float& obj_y, float& obj_width, float& obj_height)
{
	//assuming object has width and height of 30 and it is centered
	float tile_width = 30;
	float tile_height = 30;
	
	float objLeftX = obj_x;
	float objRightX = obj_x + obj_width;
	float objTopY = obj_y;
	float objBottomY = obj_y + obj_height;
	
	std::uint32_t rectLeftX = tile_x;
	std::uint32_t rectRightX = tile_x + tile_width;
	std::uint32_t rectTopY = tile_y;
	std::uint32_t rectBottomY = tile_y + tile_height;
	
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
