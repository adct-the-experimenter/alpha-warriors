#include "AttackPowerMechanicSystem.h"

#include "core/system.h"

#include "core/coordinator.h"

#include <iostream>

#include <cmath>


extern Coordinator gCoordinator;



void AttackPowerMechanicSystem::Init(std::uint8_t num_players)
{
	for(size_t i = 0; i < MAX_NUMBER_OF_ENTITIES_IN_GAME; i++)
	{
		entity_attack_boxes_ptrs[i] = nullptr;
		entity_health_ptrs[i] = nullptr;
		entity_position_ptrs[i] = nullptr;
		entity_last_hit_by_ptrs[i] = nullptr;
		entity_alive_ptrs[i] = nullptr;
		entity_taking_damage_state_ptrs[i] = nullptr;
		entity_attack_damage_factor_ptrs[i] = nullptr;
		entity_hurt_invincible_ptrs[i] = nullptr;
		entity_sound_comp_types[i] = nullptr;
		
		entity_knockback[i] = nullptr;
	}
	
	size_t iterator = 0;
	
	for (auto const& entity : mEntities)
	{
		auto& gen_entity_state = gCoordinator.GetComponent<GeneralEnityState>(entity);
		auto& reg_attacker = gCoordinator.GetComponent<RegularAttacker>(entity);
		auto& transform = gCoordinator.GetComponent<Transform2D>(entity);
		auto& sound_comp = gCoordinator.GetComponent<SoundComponent>(entity);
		
		entity_health_ptrs[iterator] = &gen_entity_state.health;
		
		entity_attack_boxes_ptrs[iterator] = &reg_attacker.attack_box;
		
		entity_position_ptrs[iterator] = &transform.position;
				
		entity_last_hit_by_ptrs[iterator] = &gen_entity_state.last_hit_by_entity_num;
		
		entity_alive_ptrs[iterator] = &gen_entity_state.alive;
		
		entity_taking_damage_state_ptrs[iterator] = &gen_entity_state.taking_damage;
		
		entity_hurt_invincible_ptrs[iterator] = &gen_entity_state.hurt_invincible;
		
		entity_attack_damage_factor_ptrs[iterator] = &gen_entity_state.damage_factor;
		
		//set collected power for player
		gen_entity_state.collected_powers[gen_entity_state.current_power] = 1;
		
		entity_sound_comp_types[iterator] = &sound_comp;
		
		entity_knockback[iterator] = &gen_entity_state.victim_knockback_amt;
		
		iterator++;
	}
	
	m_num_players = num_players;
}



static float speed_boost = 20.0f;

static float teleport_speed_boost = 5.0f;

static float strong_hit_factor = 1.5f;
static float weak_hit_factor = 0.7f;

void AttackPowerMechanicSystem::HandlePowerActivation(float& dt)
{
	
	for (auto const& entity : mEntities)
	{
		
		auto& transform = gCoordinator.GetComponent<Transform2D>(entity);
		auto& rigidBody = gCoordinator.GetComponent<RigidBody2D>(entity);
		auto& animation = gCoordinator.GetComponent<Animation>(entity);
		auto& collisionBox = gCoordinator.GetComponent<CollisionBox>(entity);
		auto& gen_entity_state = gCoordinator.GetComponent<GeneralEnityState>(entity);
		auto& reg_attacker = gCoordinator.GetComponent<RegularAttacker>(entity);
		
		//skip if not alive
		if(gen_entity_state.actor_state == EntityState::DEAD){continue;}
		
		bool reg_attack_button_released = false;
		
		//if entity pressed attack button and isn't in hurting state, or attacking, and alive.
		if(gen_entity_state.regularAttackButtonPressed && !gen_entity_state.regularAttackButtonHeld
		&& !gen_entity_state.powerButtonPressed
		&& !gen_entity_state.energyButtonPressed && !gen_entity_state.energyButtonHeld 
		&& gen_entity_state.alive 
		&& !gen_entity_state.taking_damage 
		&& gen_entity_state.actor_state != EntityState::HURTING_KNOCKBACK
		&& gen_entity_state.actor_state != EntityState::ATTACKING_NO_MOVE)
		{
			reg_attack_button_released = true;
						
			if(reg_attacker.state == PhysicalAttackerState::READY_FOR_STRONG_HIT)
			{
				reg_attacker.state = PhysicalAttackerState::LAUNCH_STRONG_HIT;		
				gen_entity_state.damage_factor *= strong_hit_factor;
			}
			else if(reg_attacker.state == PhysicalAttackerState::IDLE ||
					reg_attacker.state == PhysicalAttackerState::CHARGING)
			{
				reg_attacker.state = PhysicalAttackerState::LAUNCH_WEAK_HIT;
				
				gen_entity_state.damage_factor *= weak_hit_factor;
			}
						
			//if attack box is not active i.e. player is not attacking already
			if(!reg_attacker.attack_box.active)
			{
				animation.attackMode = 0;
			
				//initialize attack box
				
				reg_attacker.attack_box.active = true;
				gen_entity_state.actor_state = EntityState::ATTACKING_NO_MOVE;
			}
			
			gen_entity_state.time_reg_attack_button_held = 0.0f;
			gen_entity_state.regularAttackButtonPressed = false;
			gen_entity_state.regularAttackButtonHeld = false;
			
		}
		
		//if entity holds attack button 
		if(!gen_entity_state.regularAttackButtonPressed && gen_entity_state.regularAttackButtonHeld
		&& !reg_attack_button_released
		&& !gen_entity_state.powerButtonPressed  
		&& !gen_entity_state.energyButtonPressed && !gen_entity_state.energyButtonHeld
		&& !gen_entity_state.taking_damage 
		&& gen_entity_state.actor_state != EntityState::HURTING_KNOCKBACK
		&& gen_entity_state.actor_state != EntityState::ATTACKING_NO_MOVE)
		{
			reg_attacker.attack_button_released = false;
			
			//if regular attack button held down for 1 second
			if(gen_entity_state.time_reg_attack_button_held >= 1.5f)
			{
				reg_attacker.state = PhysicalAttackerState::READY_FOR_STRONG_HIT;
			}
			else
			{
				gen_entity_state.time_reg_attack_button_held += dt;
				reg_attacker.state = PhysicalAttackerState::CHARGING;
			}
			
		}
		
		//change and/or activate current power based on input
		if(!gen_entity_state.regularAttackButtonPressed && !gen_entity_state.regularAttackButtonHeld
		&& !gen_entity_state.energyButtonPressed && !gen_entity_state.energyButtonHeld 
		&& gen_entity_state.powerButtonPressed 
		&& gen_entity_state.requested_power < 8 && gen_entity_state.alive && !gen_entity_state.taking_damage 
		&& gen_entity_state.actor_state != EntityState::HURTING_KNOCKBACK
		&& gen_entity_state.actor_state != EntityState::ATTACKING_NO_MOVE)
		{
			
			//std::cout << "Player " << int(player.player_num) << " requested this power: " << int(player.requested_power) << std::endl;
			//check which power player is requesting
			//change to power requested if player has this power
			if( gen_entity_state.requested_power != -1 && gen_entity_state.collected_powers[gen_entity_state.requested_power])
			{
				//activate power, if not active
				if( !gen_entity_state.powers_activated[gen_entity_state.requested_power])
				{
					gen_entity_state.current_power = gen_entity_state.requested_power;
					gen_entity_state.actor_state = EntityState::ATTACKING_NO_MOVE;
					
				}
				
			}
			else
			{
				//reset back to the power that the player has used previously saved in current power.
				gen_entity_state.requested_power = gen_entity_state.current_power;
			}
			
			//activate power if not activated
			if( !gen_entity_state.powers_activated[gen_entity_state.current_power])
			{
				gen_entity_state.actor_state = EntityState::ATTACKING_NO_MOVE;
				animation.attackMode = gen_entity_state.current_power + 1;
				
				//std::cout << "current power of player " << int(player.player_num) << ": " << int(player.current_power) << std::endl;
				gen_entity_state.powers_activated[gen_entity_state.current_power] = 1;
				
				//set attack box or player speed based on power
				switch(gen_entity_state.current_power)
				{
					// sneak
					case 0:
					{
						//do nothing, no attack box.
						break;
					}
					//dash
					case 1:
					{
						//increase horizontal speed
						rigidBody.velocity.x = speed_boost*rigidBody.velocity.x;
						
						//cancel any attack if dashing
						reg_attacker.attack_box.active = false;
						
						break;
					}
					//shield
					case 2:
					{
						//activate collision box
						reg_attacker.attack_box.active = true;
						
						reg_attacker.attack_box.collisionBox.x = -5;
						reg_attacker.attack_box.collisionBox.y = -5;
						//it won't collide with another attack box
						
						break;
					}
					//chunks
					case 3:
					{
						reg_attacker.attack_box.active = true;

						//decrease horizontal speed
						rigidBody.velocity.x = 0.5*rigidBody.velocity.x;
						break;
					}
					//big
					case 4:
					{
						reg_attacker.attack_box.active = true;
						
						//move player up
						transform.position.y -= 60;
						
						//change player collision box to match the large size
						collisionBox.width = 60;
						collisionBox.height = 120;
						
						//set horizontal speed to zero
						rigidBody.velocity.x = 0.0f;
						break;
					}
				}
			}
			
			//reset power button pressed
			gen_entity_state.powerButtonPressed = false;
			
		}
		
		
		
		//cool down timers
		
		//regular attack cooldown
		if(reg_attacker.attack_box.active)
		{
			gen_entity_state.regular_attack_cooldown_timer_val += dt;
			
			//if player is not in the air, stop player from moving horizontally
			if(!rigidBody.velocity.y){rigidBody.velocity.x = 0.0f;}
			
			//if more than 0.5 seconds has passed
			if(gen_entity_state.regular_attack_cooldown_timer_val >= 0.6f
				&& reg_attacker.state == PhysicalAttackerState::LAUNCH_STRONG_HIT)
			{
				//reset attackbox active
				reg_attacker.attack_box.active = false;
				//reset timer value
				gen_entity_state.regular_attack_cooldown_timer_val = 0;
				//reset animation
				animation.attackMode = -1;
				
				gen_entity_state.actor_state = EntityState::NONE;
				reg_attacker.state = PhysicalAttackerState::IDLE;
				gen_entity_state.damage_factor /= strong_hit_factor;
			}
			
			if(gen_entity_state.regular_attack_cooldown_timer_val >= 0.25f
				&& reg_attacker.state == PhysicalAttackerState::LAUNCH_WEAK_HIT)
			{
				//reset attackbox active
				reg_attacker.attack_box.active = false;
				//reset timer value
				gen_entity_state.regular_attack_cooldown_timer_val = 0;
				//reset animation
				animation.attackMode = -1;
				
				gen_entity_state.actor_state = EntityState::NONE;
				reg_attacker.state = PhysicalAttackerState::IDLE;
				gen_entity_state.damage_factor /= weak_hit_factor;
			}
		}
		
		//cooldown for special power if activated
		for(size_t i = 0; i < 8; i++)
		{
			
			//if special power is active
			if(gen_entity_state.powers_activated.test(i))
			{
				
				gen_entity_state.sp_attack_cooldown_timer_val_array[i] += dt;
								
				//reset attack box or player speed based on power
				switch(i)
				{
					// sneak
					case 0:
					{
						
						//if more than 2 seconds have passed
						if(gen_entity_state.sp_attack_cooldown_timer_val_array[i] >= 3)
						{
							//reset bitset for power activated if cooldown time has finished
							gen_entity_state.powers_activated[i] = 0;
							//reset cooldown timer value
							gen_entity_state.sp_attack_cooldown_timer_val_array[i] = 0;
							//reset animation for attack mode
							animation.attackMode = -1;
							
							gen_entity_state.actor_state = EntityState::NONE;
						}
						break;
					}
					//dash
					case 1:
					{
						//return to original horizontal speed after 1 second
						if(gen_entity_state.sp_attack_cooldown_timer_val_array[i] >= 1)
						{
							rigidBody.velocity.x = (1/speed_boost)*rigidBody.velocity.x;
							
							//reset bitset for power activated if cooldown time has finished
							gen_entity_state.powers_activated[i] = 0;
							//reset cooldown timer value
							gen_entity_state.sp_attack_cooldown_timer_val_array[i] = 0;
							//reset animation for attack mode
							animation.attackMode = -1;
							
							gen_entity_state.actor_state = EntityState::NONE;
						}
						
						
						break;
					}
					//shield
					case 2:
					{
						//if more than 4 seconds have passed
						if(gen_entity_state.sp_attack_cooldown_timer_val_array[i] >= 4)
						{
							reg_attacker.attack_box.active = false;
							
							//reset bitset for power activated if cooldown time has finished
							gen_entity_state.powers_activated[i] = 0;
							//reset cooldown timer value
							gen_entity_state.sp_attack_cooldown_timer_val_array[i] = 0;
							
							//reset animation for attack mode
							animation.attackMode = -1;
							
							gen_entity_state.actor_state = EntityState::NONE;
							
						}
						
						break;
					}
					//chunks
					case 3:
					{
						//if more than 4 seconds have passed
						if(gen_entity_state.sp_attack_cooldown_timer_val_array[i] >= 4)
						{
							//decrease horizontal speed
							rigidBody.velocity.x = 2*rigidBody.velocity.x;
							
							reg_attacker.attack_box.active = false;
							
							//reset bitset for power activated if cooldown time has finished
							gen_entity_state.powers_activated[i] = 0;
							//reset cooldown timer value
							gen_entity_state.sp_attack_cooldown_timer_val_array[i] = 0;
							
							//reset animation for attack mode
							animation.attackMode = -1;
							
							gen_entity_state.actor_state = EntityState::NONE;
							
						}
						
						break;
					}
					//big
					case 4:
					{
						//if more than 5 seconds have passed
						if(gen_entity_state.sp_attack_cooldown_timer_val_array[i] >= 1)
						{
							
							if(gen_entity_state.sp_attack_cooldown_timer_val_array[i] >= 4)
							{
								//reset bitset for power activated if cooldown time has finished
								gen_entity_state.powers_activated[i] = 0;
								//reset cooldown timer value
								gen_entity_state.sp_attack_cooldown_timer_val_array[i] = 0;
							}
							else
							{
								//reset speed
								rigidBody.velocity.x = rigidBody.velocity.x;
								
								
								//change player collision box to match the normal size
								collisionBox.width = 30;
								collisionBox.height = 60;
								
								reg_attacker.attack_box.active = false;
								
								
								//reset animation for attack mode
								animation.attackMode = -1;
								
								gen_entity_state.actor_state = EntityState::NONE;
							}
							
							
							
							
							
						}
						
						break;
					}
				}
				
			}
		}
		
		//activation and cooldown for teleport button if activated
		if(gen_entity_state.teleportButton && !gen_entity_state.taking_damage )
		{			
			gen_entity_state.inTeleportMode = true;
			
			//rigidBody.in_flying_state = true;
			
			//make player fast
			rigidBody.velocity.x = teleport_speed_boost*rigidBody.velocity.x;
			if(rigidBody.in_flying_state){rigidBody.velocity.y = teleport_speed_boost*rigidBody.velocity.y;}
			
			//make player invisible
			animation.attackMode = 1;
			
			gen_entity_state.teleport_cooldown_timer_val += dt;
			
			if(gen_entity_state.teleport_cooldown_timer_val >= 0.5f)
			{
				PlayGeneralSound(GeneralSoundID::TELEPORT_ONE);
				rigidBody.velocity.x = (1/teleport_speed_boost)*rigidBody.velocity.x;
				rigidBody.velocity.y = (1/teleport_speed_boost)*rigidBody.velocity.y;
				//reset animation for attack mode
				animation.attackMode = -1;
			
				gen_entity_state.actor_state = EntityState::NONE;
				
				if(gen_entity_state.teleport_cooldown_timer_val >= 1.0f){gen_entity_state.teleport_cooldown_timer_val = 0.0f;}
			}
			
			
		}
		else if(!gen_entity_state.teleportButton && !gen_entity_state.taking_damage)
		{
			gen_entity_state.inTeleportMode = false;
			
			if(gen_entity_state.teleport_cooldown_timer_val > 0.0f)
			{
				PlayGeneralSound(GeneralSoundID::TELEPORT_ONE);
				
				if(gen_entity_state.teleport_cooldown_timer_val >= 1.0f)
				{
					gen_entity_state.teleport_cooldown_timer_val = 0.0f;
					gen_entity_state.inTeleportMode = false;
				}
				
				rigidBody.velocity.x = (1/teleport_speed_boost)*rigidBody.velocity.x;
				rigidBody.velocity.y = (1/teleport_speed_boost)*rigidBody.velocity.y;
				//reset animation for attack mode
				animation.attackMode = -1;
			
				gen_entity_state.actor_state = EntityState::NONE;
			}
			
			gen_entity_state.teleport_cooldown_timer_val = 0.0f;
		}
		else
		{
			gen_entity_state.inTeleportMode = false;
			gen_entity_state.teleport_cooldown_timer_val = 0.0f;
		}
		
	}
	
}



static bool PlayerCollisionWithRectangleDetected(Rectangle& rect,
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


void AttackPowerMechanicSystem::MoveAttackBoxesWithPlayer(float& dt)
{
	for (auto const& entity : mEntities)
	{
		
		auto& transform = gCoordinator.GetComponent<Transform2D>(entity);
		auto& gen_entity_state = gCoordinator.GetComponent<GeneralEnityState>(entity);
		auto& reg_attacker = gCoordinator.GetComponent<RegularAttacker>(entity);
		auto& animation = gCoordinator.GetComponent<Animation>(entity);
		
		if(reg_attacker.attack_box.active)
		{
			switch(animation.attackMode)
			{
				//regular attack
				case 0:
				{
					float offset_x = 0;
			
					if(animation.face_dir == FaceDirection::EAST)
					{
						offset_x = gen_entity_state.attack_box_offset;
					}
					else if(animation.face_dir == FaceDirection::WEST)
					{
						offset_x = -gen_entity_state.attack_box_offset;
					}
					
					//activate collision box
					reg_attacker.attack_box.collisionBox.x = transform.position.x + offset_x; 
					reg_attacker.attack_box.collisionBox.y = transform.position.y + 20;
					reg_attacker.attack_box.collisionBox.width = 30;
					reg_attacker.attack_box.collisionBox.height = 30;
					
					reg_attacker.attack_box.player_num = gen_entity_state.entity_num;
					
					break;
				}
				//chunks
				case 4:
				{
					int offset_x = 0;
			
					if(animation.face_dir == FaceDirection::EAST)
					{
						offset_x = 10;
					}
					else if(animation.face_dir == FaceDirection::WEST)
					{
						offset_x = -10;
					}
					
					//activate collision box
					reg_attacker.attack_box.collisionBox.x = transform.position.x + offset_x; 
					reg_attacker.attack_box.collisionBox.y = transform.position.y ;
					reg_attacker.attack_box.collisionBox.width = 50;
					reg_attacker.attack_box.collisionBox.height = 50;
					
					reg_attacker.attack_box.player_num = gen_entity_state.entity_num;
					break;
				}
				//big
				case 5:
				{
					int offset_x = 0;
			
					if(animation.face_dir == FaceDirection::EAST)
					{
						offset_x = 10;
					}
					else if(animation.face_dir == FaceDirection::WEST)
					{
						offset_x = -10;
					}
					
					//activate attack collision box
					reg_attacker.attack_box.collisionBox.x = transform.position.x + offset_x; 
					reg_attacker.attack_box.collisionBox.y = transform.position.y ;
					reg_attacker.attack_box.collisionBox.width = 60;
					reg_attacker.attack_box.collisionBox.height = 120;
					
					reg_attacker.attack_box.player_num = gen_entity_state.entity_num;
				}
				default:{break;}
			}
			
			
		}
		
		
		
	}
	
}

void AttackPowerMechanicSystem::CollisionDetectionBetweenPlayers()
{
	//check all collisions for active entities
	for(uint ent_it = 1; ent_it < MAX_NUMBER_OF_ENTITIES_IN_GAME; ent_it++)
	{
		for(uint i = 0; i < ent_it;i++)
		{			
			int entity_a_num = i;
			int entity_b_num = ent_it;
			AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(entity_a_num,entity_b_num);
		}
	}
	
}

bool AttackPowerMechanicSystem::AreBothPlayersAlive(int& player_a_num, int& player_b_num)
{
	if(!entity_alive_ptrs[player_a_num] || !entity_alive_ptrs[player_b_num])
	{
		return false;
	}
	
	else if(*entity_alive_ptrs[player_a_num] && *entity_alive_ptrs[player_b_num])
	{
		return true;
	}
	
	return false;
}

AttackEvent AttackPowerMechanicSystem::CheckCollisionBetween2Players(int& player_a_num, int& player_b_num)
{
	AttackEvent attack_event;
	attack_event.attack = false;
	attack_event.player_num_victim = 0;
	attack_event.player_num_attacker = 0;
	
	if(!AttackPowerMechanicSystem::AreBothPlayersAlive(player_a_num,player_b_num))
	{
		return attack_event;
	}
	
	AttackBox* playerA_attackbox_ptr = entity_attack_boxes_ptrs[player_a_num];
	AttackBox* playerB_attackbox_ptr = entity_attack_boxes_ptrs[player_b_num];
	
	Vector2* playerA_position_ptr = entity_position_ptrs[player_a_num];
	Vector2* playerB_position_ptr = entity_position_ptrs[player_b_num];
	
	//player width and height
	std::uint32_t width = 30;
	std::uint32_t height = 80;

	//if this player A attack box is active, player B attack box inactive
	if( playerA_attackbox_ptr->active && !playerB_attackbox_ptr->active)
	{
		if(PlayerCollisionWithRectangleDetected(playerA_attackbox_ptr->collisionBox,
												playerB_position_ptr->x, playerB_position_ptr->y, width,height)
		   )
		{
			
			
			attack_event.attack = true;
			attack_event.player_num_victim = player_b_num;
			attack_event.player_num_attacker = player_a_num;
			
		}
	}
	//if player B attack box is active, player A attack box inactive
	else if(playerB_attackbox_ptr->active && !playerA_attackbox_ptr->active)
	{
		if(PlayerCollisionWithRectangleDetected(playerB_attackbox_ptr->collisionBox,
												playerA_position_ptr->x, playerA_position_ptr->y, width,height)
		   )
		{
			attack_event.attack = true;
			attack_event.player_num_victim = player_a_num;
			attack_event.player_num_attacker = player_b_num;
			
		}
	}
	
	return attack_event;
}

static const int16_t base_health_reduction_factor = 10;

void AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(int& player_a_num, int& player_b_num)
{
	AttackEvent attack_event;
	attack_event = AttackPowerMechanicSystem::CheckCollisionBetween2Players(player_a_num, player_b_num);
	
	//if attack happened, and victim is not already in state of taking damage or in hurt invicible state
	if(attack_event.attack && !*entity_taking_damage_state_ptrs[attack_event.player_num_victim] && !*entity_hurt_invincible_ptrs[attack_event.player_num_victim] )
	{
		//std::cout << "Player " << int(attack_event.player_num_attacker) << " took away 10 HP from player " << int(attack_event.player_num_victim) << std::endl;
		
		//decrease health of victim player
		*entity_health_ptrs[attack_event.player_num_victim] -= base_health_reduction_factor*(*entity_attack_damage_factor_ptrs[attack_event.player_num_attacker]);
		//set last hit by player variable for victim player
		*entity_last_hit_by_ptrs[attack_event.player_num_victim] = attack_event.player_num_attacker;
		
		*entity_taking_damage_state_ptrs[attack_event.player_num_victim] = true;
		
		//knock back
		float tiles_knock = 2.0f;
		float knockback = tiles_knock*(*entity_attack_damage_factor_ptrs[attack_event.player_num_attacker]);
		float sign_x = 1;
		float sign_y = 1;
		
		//if victim is to the left of attacker
		if(entity_position_ptrs[attack_event.player_num_victim]->x < 
			entity_position_ptrs[attack_event.player_num_attacker]->x)
		{
			sign_x = -1;
		}
		
		//if victim is above the player
		if(entity_position_ptrs[attack_event.player_num_victim]->y < 
			entity_position_ptrs[attack_event.player_num_attacker]->y)
		{
			sign_y = -1;
		}
		
		entity_knockback[attack_event.player_num_victim]->x = sign_x*knockback;
		entity_knockback[attack_event.player_num_victim]->y = sign_y*knockback;
		
		//make sound
		//player_sound_comp_types[attack_event.player_num_victim]->sound_type = SoundType::GENERAL_SOUND;
		//player_sound_comp_types[attack_event.player_num_victim]->general_sound_id = GeneralSoundID::HIT_SOUND;
		
		entity_sound_comp_types[attack_event.player_num_attacker]->sound_type = SoundType::CHAR_SOUND;
		entity_sound_comp_types[attack_event.player_num_attacker]->char_sound_id = CharSoundID::HIT_SOUND;
	}

}


static const float knockback_factor = 90; //pixels per frame

void AttackPowerMechanicSystem::ReactToCollisions(float& dt)
{
	for(auto const& entity: mEntities)
	{
		
		auto& rigidBody = gCoordinator.GetComponent<RigidBody2D>(entity);
		auto& animation = gCoordinator.GetComponent<Animation>(entity);
		auto& gen_entity_state = gCoordinator.GetComponent<GeneralEnityState>(entity);
		
		//if player is in state of taking damage 
		//activate hurt animation and keep them from moving
		if(gen_entity_state.taking_damage)
		{
			gen_entity_state.taking_damage = false;
			gen_entity_state.hurt_invincible = true;
			
			//reset animation
			animation.attackMode = -1;
			animation.frame_count = 0;
			animation.hurt = true;
			
						
			gen_entity_state.hurt_anim_time_count = 0;
			gen_entity_state.hurt_anim_time_count += dt;
			
			//put in hurt state
			gen_entity_state.actor_state = EntityState::HURTING_KNOCKBACK;
		}
		else
		{
			if(gen_entity_state.actor_state == EntityState::HURTING_KNOCKBACK)
			{
				//knock back
				rigidBody.velocity.x = knockback_factor*entity_knockback[gen_entity_state.entity_num]->x;
				rigidBody.velocity.y = knockback_factor*entity_knockback[gen_entity_state.entity_num]->y;
				
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

static bool debugRenderAttackBox = false;
static bool debugDrawPowerRequest = false;

void AttackPowerMechanicSystem::DebugRender()
{
	for (auto const& entity : mEntities)
	{
		auto& reg_attacker = gCoordinator.GetComponent<RegularAttacker>(entity);
		auto& gen_entity_state = gCoordinator.GetComponent<GeneralEnityState>(entity);
		
		if(reg_attacker.attack_box.active && debugRenderAttackBox)
		{
			DrawRectangleRec(reg_attacker.attack_box.collisionBox, RED);
		}
		
		
		
		if(debugDrawPowerRequest)
		{
			std::string req_num_str = std::to_string(gen_entity_state.requested_power);
			DrawText(req_num_str.c_str(), 100 + 20*gen_entity_state.entity_num, 20, 20, RED); 
		}
		
	}
}


void AttackPowerMechanicSystem::HandleCollisionBetweenPlayerAttacksAndWorldTiles()
{
	//world
	World* world_ptr = &world_one;
	
	//calculate tile that object is on
	
	//for each player attack box
	for (auto const& entity : mEntities)
	{
		
		auto& gen_entity_state = gCoordinator.GetComponent<GeneralEnityState>(entity);
		auto& phy_type_comp = gCoordinator.GetComponent<PhysicsTypeComponent>(entity);
		auto& reg_attacker = gCoordinator.GetComponent<RegularAttacker>(entity);
		
		float obj_x, obj_y, obj_width, obj_height;
		
		//use attack box for tile position if player is attacking
		if(gen_entity_state.actor_state == EntityState::ATTACKING_NO_MOVE)
		{
			obj_x = reg_attacker.attack_box.collisionBox.x;
			obj_y = reg_attacker.attack_box.collisionBox.y;
			obj_width = reg_attacker.attack_box.collisionBox.width;
			obj_height = reg_attacker.attack_box.collisionBox.height;
		}
		else
		{
			continue; //skip to next iteration
		}
		
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

		size_t limit_tile_search = 0;
		
		//do not process bottom tiles if player is moving upward or grounded
		//done to not destroy tiles below player
		if(phy_type_comp.grounded)
		{
			//std::cout << "Tile not destroyed!\n";
			limit_tile_search = 6;
		}
		
		//for tiles around player
		for(size_t i = 0; i < tiles_around_object.size() - limit_tile_search; i++)
		{
			size_t& tile_index = tiles_around_object[i];
			
			
			if(world_ptr->tiles_vector[tile_index].type == TileType::PUSH_BACK)
			{
				//if player(obj) collides with a platforms
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
			else if(world_ptr->tiles_vector[tile_index].type == TileType::PLANET_DESTRUCTION)
			{
				//if player(obj) collides with a tile
				if(CollisionWithTileDetected(world_ptr->tiles_vector[tile_index].x,world_ptr->tiles_vector[tile_index].y,
								   obj_x, obj_y, obj_width, obj_height) 
					)
				{
					
					world_ptr->planet_destruction_start = true;
				}
			}

		}
	}
	
	
}
