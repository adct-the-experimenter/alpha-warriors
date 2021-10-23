#include "ReactionSystem.h"

#include "core/system.h"

#include "core/coordinator.h"

#include <iostream>

extern Coordinator gCoordinator;

static const float knockback_factor = 90; //pixels per frame

void ReactionSystem::HandleReactionToCollisions(float& dt)
{
	for(auto const& entity: mEntities)
	{
		
		auto& rigidBody = gCoordinator.GetComponent<RigidBody2D>(entity);
		auto& animation = gCoordinator.GetComponent<Animation>(entity);
		auto& gen_entity_state = gCoordinator.GetComponent<GeneralEnityState>(entity);
		
		//if player is in state of taking damage 
		//activate hurt animation
		if(gen_entity_state.taking_damage)
		{
			gen_entity_state.taking_damage = false;
			gen_entity_state.hurt_invincible = true;
			
			//reset animation
			animation.attackMode = -1;
			animation.frame_count = 0;
			animation.hurt = true;
			
						
			gen_entity_state.hurt_anim_time_count = 0.0f;
			gen_entity_state.hurt_anim_time_count += dt;
			
			//put in hurt state
			gen_entity_state.actor_state = EntityState::HURTING_KNOCKBACK;
		}
		else
		{
			if(gen_entity_state.actor_state == EntityState::HURTING_KNOCKBACK)
			{
				//knock back
				rigidBody.velocity.x = knockback_factor*gen_entity_state.victim_knockback_amt.x;
				rigidBody.velocity.y = knockback_factor*gen_entity_state.victim_knockback_amt.y;
				
				gen_entity_state.hurt_anim_time_count += dt;
			
				//if 0.5 seconds seconds have passed, stop hurt animation
				if(gen_entity_state.hurt_anim_time_count >= 0.5f)
				{
					animation.hurt = false;
					gen_entity_state.hurt_anim_time_count = 0;
					gen_entity_state.actor_state = EntityState::NONE;
					gen_entity_state.hurt_invincible = false;
				}
			}
			
			
		}
		
	}
}
