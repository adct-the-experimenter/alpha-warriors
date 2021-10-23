#ifndef CRAFTING_SYSTEM_H
#define CRAFTING_SYSTEM_H

#include "core/system.h"

#include "misc/level_maps.h"

class CraftingSystem : public System
{
public:
	void Init();
	
	void HandleCrafting();
	
private:
	void HandleTilePlacementInWorld(World* world_ptr);
};

#endif
