#include "AttackPowerMechanicSystem.h"

#include "core/system.h"

#include "core/coordinator.h"

#include <iostream>

#include <cmath>


extern Coordinator gCoordinator;


void AttackPowerMechanicSystem::Init(std::uint8_t num_players)
{
	for(size_t i = 0; i < 8; i++)
	{
		player_attack_boxes_ptrs[i] = nullptr;
		player_health_ptrs[i] = nullptr;
		player_position_ptrs[i] = nullptr;
		player_last_hit_by_ptrs[i] = nullptr;
		player_alive_ptrs[i] = nullptr;
		player_taking_damage_state_ptrs[i] = nullptr;
		player_attack_damage_factor_ptrs[i] = nullptr;
		player_hurt_invincible_ptrs[i] = nullptr;
		player_sound_comp_types[i] = nullptr;
		
		player_knockback[i] = nullptr;
	}
		
	for (auto const& entity : mEntities)
	{
		auto& player = gCoordinator.GetComponent<Player>(entity);
		auto& gen_entity_state = gCoordinator.GetComponent<GeneralEnityState>(entity);
		auto& transform = gCoordinator.GetComponent<Transform2D>(entity);
		auto& sound_comp = gCoordinator.GetComponent<SoundComponent>(entity);
		
		player_health_ptrs[player.player_num - 1] = &gen_entity_state.health;
		
		player_attack_boxes_ptrs[player.player_num - 1] = &player.attack_box;
		
		player_position_ptrs[player.player_num - 1] = &transform.position;
				
		player_last_hit_by_ptrs[player.player_num - 1] = &player.last_hit_by_player_num;
		
		player_alive_ptrs[player.player_num - 1] = &player.alive;
		
		player_taking_damage_state_ptrs[player.player_num - 1] = &gen_entity_state.taking_damage;
		
		player_hurt_invincible_ptrs[player.player_num - 1] = &gen_entity_state.hurt_invincible;
		
		player_attack_damage_factor_ptrs[player.player_num - 1] = &player.damage_factor;
		
		//set collected power for player
		player.collected_powers[player.current_power] = 1;
		
		player_sound_comp_types[player.player_num - 1] = &sound_comp;
		
		player_knockback[player.player_num - 1] = &gen_entity_state.victim_knockback_amt;
	}
	
	m_num_players = num_players;
}



static float speed_boost = 20.0f;

void AttackPowerMechanicSystem::HandlePowerActivation(float& dt)
{
	
	for (auto const& entity : mEntities)
	{
		
		auto& transform = gCoordinator.GetComponent<Transform2D>(entity);
		auto& rigidBody = gCoordinator.GetComponent<RigidBody2D>(entity);
		auto& player = gCoordinator.GetComponent<Player>(entity);
		auto& animation = gCoordinator.GetComponent<Animation>(entity);
		auto& collisionBox = gCoordinator.GetComponent<CollisionBox>(entity);
		auto& gen_entity_state = gCoordinator.GetComponent<GeneralEnityState>(entity);
		
		//if player pressed attack button and isn't in hurting state and alive.
		if(player.regularAttackButtonPressed && player.alive && !gen_entity_state.taking_damage && player.state != PlayerState::HURTING)
		{
			//if attack box is not active i.e. player is not attacking already
			if(!player.attack_box.active)
			{
				animation.attackMode = 0;
			
				//initialize attack box
				
				player.attack_box.active = true;
				player.state = PlayerState::ATTACKING;
				gen_entity_state.actor_state = EntityState::ATTACKING_NO_MOVE;
			}
			
			player.regularAttackButtonPressed = false;
		}
		
		//change and/or activate current power based on input
		else if(player.powerButtonPressed && player.requested_power < 8 && player.alive && !gen_entity_state.taking_damage && player.state != PlayerState::HURTING)
		{
			
			//std::cout << "Player " << int(player.player_num) << " requested this power: " << int(player.requested_power) << std::endl;
			//check which power player is requesting
			//change to power requested if player has this power
			if( player.requested_power != -1 && player.collected_powers[player.requested_power])
			{
				//activate power, if not active
				if( !player.powers_activated[player.requested_power])
				{
					player.current_power = player.requested_power;
					player.state = PlayerState::ATTACKING;
					
				}
				
				
			}
			else
			{
				//reset back to the power that the player has used previously saved in current power.
				player.requested_power = player.current_power;
			}
			
			//activate power if not activated
			if( !player.powers_activated[player.current_power])
			{
				player.state = PlayerState::ATTACKING;
				animation.attackMode = player.current_power + 1;
				
				//std::cout << "current power of player " << int(player.player_num) << ": " << int(player.current_power) << std::endl;
				player.powers_activated[player.current_power] = 1;
				
				//set attack box or player speed based on power
				switch(player.current_power)
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
						player.attack_box.active = false;
						
						break;
					}
					//shield
					case 2:
					{
						//activate collision box
						player.attack_box.active = true;
						
						player.attack_box.collisionBox.x = -5;
						player.attack_box.collisionBox.y = -5;
						//it won't collide with another attack box
						
						break;
					}
					//chunks
					case 3:
					{
						player.attack_box.active = true;

						//decrease horizontal speed
						rigidBody.velocity.x = 0.5*rigidBody.velocity.x;
						break;
					}
					//big
					case 4:
					{
						player.attack_box.active = true;
						
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
			player.powerButtonPressed = false;
			
		}
		
		//launch small energy beam if energy beam button pressed, and player is not taking damage
		if(player.energyButtonPressed && !gen_entity_state.taking_damage)
		{
			auto& energy_attacker = gCoordinator.GetComponent<EnergyAttacker>(entity);
			
			energy_attacker.send_energy_beam = true;
			
			player.energyButtonPressed = false;
			
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
		
		if(player.energyButtonHeld && !gen_entity_state.taking_damage)
		{
			player.time_energy_button_held += dt;
			
			//if energy button held down for 2 seconds
			if(player.time_energy_button_held >= 2.0f)
			{
				//activate large energy blast
				player.time_energy_button_held = 0.0f;
				
				auto& energy_attacker = gCoordinator.GetComponent<EnergyAttacker>(entity);
				
				energy_attacker.energy_blast = true;
											
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
		
		//cool down timers
		
		//regular attack cooldown
		if(player.attack_box.active)
		{
			player.regular_attack_cooldown_timer_val += dt;
			
			//if player is not in the air, stop player from moving horizontally
			if(!rigidBody.velocity.y){rigidBody.velocity.x = 0.0f;}
			
			//if more than 0.5 seconds has passed
			if(player.regular_attack_cooldown_timer_val >= 0.5f)
			{
				//reset attackbox active
				player.attack_box.active = false;
				//reset timer value
				player.regular_attack_cooldown_timer_val = 0;
				//reset animation
				animation.attackMode = -1;
				
				player.state = PlayerState::IDLE;
				gen_entity_state.actor_state = EntityState::NONE;
			}
		}
		
		//cooldown for special power if activated
		for(size_t i = 0; i < 8; i++)
		{
			
			//if special power is active
			if(player.powers_activated.test(i))
			{
				
				player.sp_attack_cooldown_timer_val_array[i] += dt;
								
				//reset attack box or player speed based on power
				switch(i)
				{
					// sneak
					case 0:
					{
						
						//if more than 2 seconds have passed
						if(player.sp_attack_cooldown_timer_val_array[i] >= 3)
						{
							//reset bitset for power activated if cooldown time has finished
							player.powers_activated[i] = 0;
							//reset cooldown timer value
							player.sp_attack_cooldown_timer_val_array[i] = 0;
							//reset animation for attack mode
							animation.attackMode = -1;
							
							player.state = PlayerState::IDLE;
						}
						break;
					}
					//dash
					case 1:
					{
						//return to original horizontal speed after 1 second
						if(player.sp_attack_cooldown_timer_val_array[i] >= 1)
						{
							rigidBody.velocity.x = (1/speed_boost)*rigidBody.velocity.x;
							
							//reset bitset for power activated if cooldown time has finished
							player.powers_activated[i] = 0;
							//reset cooldown timer value
							player.sp_attack_cooldown_timer_val_array[i] = 0;
							//reset animation for attack mode
							animation.attackMode = -1;
							
							player.state = PlayerState::IDLE;
						}
						
						
						break;
					}
					//shield
					case 2:
					{
						//if more than 4 seconds have passed
						if(player.sp_attack_cooldown_timer_val_array[i] >= 4)
						{
							player.attack_box.active = false;
							
							//reset bitset for power activated if cooldown time has finished
							player.powers_activated[i] = 0;
							//reset cooldown timer value
							player.sp_attack_cooldown_timer_val_array[i] = 0;
							
							//reset animation for attack mode
							animation.attackMode = -1;
							
							player.state = PlayerState::IDLE;
							
						}
						
						break;
					}
					//chunks
					case 3:
					{
						//if more than 4 seconds have passed
						if(player.sp_attack_cooldown_timer_val_array[i] >= 4)
						{
							//decrease horizontal speed
							rigidBody.velocity.x = 2*rigidBody.velocity.x;
							
							player.attack_box.active = false;
							
							//reset bitset for power activated if cooldown time has finished
							player.powers_activated[i] = 0;
							//reset cooldown timer value
							player.sp_attack_cooldown_timer_val_array[i] = 0;
							
							//reset animation for attack mode
							animation.attackMode = -1;
							
							player.state = PlayerState::IDLE;
							
						}
						
						break;
					}
					//big
					case 4:
					{
						//if more than 5 seconds have passed
						if(player.sp_attack_cooldown_timer_val_array[i] >= 1)
						{
							
							if(player.sp_attack_cooldown_timer_val_array[i] >= 4)
							{
								//reset bitset for power activated if cooldown time has finished
								player.powers_activated[i] = 0;
								//reset cooldown timer value
								player.sp_attack_cooldown_timer_val_array[i] = 0;
							}
							else
							{
								//reset speed
								rigidBody.velocity.x = rigidBody.velocity.x;
								
								
								//change player collision box to match the normal size
								collisionBox.width = 30;
								collisionBox.height = 60;
								
								player.attack_box.active = false;
								
								
								//reset animation for attack mode
								animation.attackMode = -1;
								
								player.state = PlayerState::IDLE;
							}
							
							
							
							
							
						}
						
						break;
					}
				}
				
			}
		}
		
		//cooldown for small energy beam
		
		
		
		
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
		auto& player = gCoordinator.GetComponent<Player>(entity);
		auto& animation = gCoordinator.GetComponent<Animation>(entity);
		
		if(player.attack_box.active)
		{
			switch(animation.attackMode)
			{
				//regular attack
				case 0:
				{
					float offset_x = 0;
			
					if(animation.face_dir == FaceDirection::EAST)
					{
						offset_x = player.attack_box_offset;
					}
					else if(animation.face_dir == FaceDirection::WEST)
					{
						offset_x = -player.attack_box_offset;
					}
					
					//activate collision box
					player.attack_box.collisionBox.x = transform.position.x + offset_x; 
					player.attack_box.collisionBox.y = transform.position.y + 20;
					player.attack_box.collisionBox.width = 30;
					player.attack_box.collisionBox.height = 30;
					
					player.attack_box.player_num = player.player_num;
					
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
					player.attack_box.collisionBox.x = transform.position.x + offset_x; 
					player.attack_box.collisionBox.y = transform.position.y ;
					player.attack_box.collisionBox.width = 50;
					player.attack_box.collisionBox.height = 50;
					
					player.attack_box.player_num = player.player_num;
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
					player.attack_box.collisionBox.x = transform.position.x + offset_x; 
					player.attack_box.collisionBox.y = transform.position.y ;
					player.attack_box.collisionBox.width = 60;
					player.attack_box.collisionBox.height = 120;
					
					player.attack_box.player_num = player.player_num;
				}
				default:{break;}
			}
			
			
		}
		
		
		
	}
	
}

void AttackPowerMechanicSystem::CollisionDetectionBetweenPlayers()
{
	int player_a_num = 0;
	int player_b_num = 0;
	
	//if there is more than 1 player
	//check player 1 and player 2 interaction
	if( m_num_players > 1)
	{
		player_a_num = 1;
		player_b_num = 2;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
	}
	
	//if there are more than 2 players
	//check player 1 and player 3, player 2 and player 3 interactions
	if( m_num_players > 2 )
	{
		AttackEvent attack_event;
		
		//check player 1 and 3 interaction
		
		player_a_num = 1;
		player_b_num = 3;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
		//check player 2 and player 3 interaction
		
		player_a_num = 2;
		player_b_num = 3;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
	}
	
	//if there are more than 3 players
	//check player 1 and player 4, player 2 and player 4, player 3 and player 4 interactions
	if( m_num_players > 3)
	{
		
		//check player 1 and player 4 interaction
		player_a_num = 1;
		player_b_num = 4;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
		//check player 2 and player 4 interaction
		player_a_num = 2;
		player_b_num = 4;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
		//check player 3 and player 4 interaction
		player_a_num = 3;
		player_b_num = 4;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
	}
	
	//if there are 5 players
	if(m_num_players > 4)
	{
		
		//check player 1 and player 5 interaction
		player_a_num = 1;
		player_b_num = 5;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
		//check player 2 and player 5 interaction
		player_a_num = 2;
		player_b_num = 5;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
		//check player 3 and player 5 interaction
		player_a_num = 3;
		player_b_num = 5;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
		//check player 4 and player 5 interaction
		player_a_num = 4;
		player_b_num = 5;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
	}
	
	//if there are 6 players
	if(m_num_players > 5)
	{
		
		//check player 1 and player 6 interaction
		player_a_num = 1;
		player_b_num = 6;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
		//check player 2 and player 6 interaction
		player_a_num = 2;
		player_b_num = 6;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
		//check player 3 and player 6 interaction
		player_a_num = 3;
		player_b_num = 6;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
		//check player 4 and player 6 interaction
		player_a_num = 4;
		player_b_num = 6;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
		//check player 5 and player 6 interaction
		player_a_num = 5;
		player_b_num = 6;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
	}
	
	//if there are 7 players
	if(m_num_players > 6)
	{		
		//check player 1 and player 7 interaction
		player_a_num = 1;
		player_b_num = 7;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
		//check player 2 and player 7 interaction
		player_a_num = 2;
		player_b_num = 7;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
		//check player 3 and player 7 interaction
		player_a_num = 3;
		player_b_num = 7;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
		//check player 4 and player 7 interaction
		player_a_num = 4;
		player_b_num = 7;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
		//check player 5 and player 7 interaction
		player_a_num = 5;
		player_b_num = 7;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
		//check player 6 and player 7 interaction
		player_a_num = 6;
		player_b_num = 7;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
	}
	//if there are 8 players
	if(m_num_players > 7)
	{
		AttackEvent attack_event;
		
		//check player 1 and player 8 interaction
		player_a_num = 1;
		player_b_num = 8;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
		//check player 2 and player 8 interaction
		player_a_num = 2;
		player_b_num = 8;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
		//check player 3 and player 8 interaction
		player_a_num = 3;
		player_b_num = 8;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
		//check player 4 and player 8 interaction
		player_a_num = 4;
		player_b_num = 8;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
		//check player 5 and player 8 interaction
		player_a_num = 5;
		player_b_num = 8;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
		//check player 6 and player 8 interaction
		player_a_num = 6;
		player_b_num = 8;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
		
		//check player 7 and player 8 interaction
		player_a_num = 7;
		player_b_num = 8;
		AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(player_a_num,player_b_num);
	}
	
}

bool AttackPowerMechanicSystem::AreBothPlayersAlive(int& player_a_num, int& player_b_num)
{
	if(*player_alive_ptrs[player_a_num - 1] && *player_alive_ptrs[player_b_num - 1])
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
	
	AttackBox* playerA_attackbox_ptr = player_attack_boxes_ptrs[player_a_num - 1];
	AttackBox* playerB_attackbox_ptr = player_attack_boxes_ptrs[player_b_num - 1];
	
	Vector2* playerA_position_ptr = player_position_ptrs[player_a_num - 1];
	Vector2* playerB_position_ptr = player_position_ptrs[player_b_num - 1];
	
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

void AttackPowerMechanicSystem::HandlePossibleCollisionBetweenPlayers(int& player_a_num, int& player_b_num)
{
	AttackEvent attack_event;
	attack_event = AttackPowerMechanicSystem::CheckCollisionBetween2Players(player_a_num, player_b_num);
	
	//if attack happened, and victim is not already in state of taking damage or in hurt invicible state
	if(attack_event.attack && !*player_taking_damage_state_ptrs[attack_event.player_num_victim - 1] && !*player_hurt_invincible_ptrs[attack_event.player_num_victim - 1] )
	{
		//std::cout << "Player " << int(attack_event.player_num_attacker) << " took away 10 HP from player " << int(attack_event.player_num_victim) << std::endl;
		
		//decrease health of victim player
		*player_health_ptrs[attack_event.player_num_victim - 1] -= 10*(*player_attack_damage_factor_ptrs[attack_event.player_num_attacker - 1]);
		//set last hit by player variable for victim player
		*player_last_hit_by_ptrs[attack_event.player_num_victim - 1] = attack_event.player_num_attacker;
		
		*player_taking_damage_state_ptrs[attack_event.player_num_victim - 1] = true;
		
		//knock back
		float tiles_knock = 2.0f;
		float knockback = tiles_knock*(*player_attack_damage_factor_ptrs[attack_event.player_num_attacker - 1]);
		float sign_x = 1;
		float sign_y = 1;
		
		//if victim is to the left of attacker
		if(player_position_ptrs[attack_event.player_num_victim - 1]->x < 
			player_position_ptrs[attack_event.player_num_attacker - 1]->x)
		{
			sign_x = -1;
		}
		
		//if victim is above the player
		if(player_position_ptrs[attack_event.player_num_victim - 1]->y < 
			player_position_ptrs[attack_event.player_num_attacker - 1]->y)
		{
			sign_y = -1;
		}
		
		player_knockback[attack_event.player_num_victim - 1]->x = sign_x*knockback;
		player_knockback[attack_event.player_num_victim - 1]->y = sign_y*knockback;
		
		//make sound
		//player_sound_comp_types[attack_event.player_num_victim - 1]->sound_type = SoundType::GENERAL_SOUND;
		//player_sound_comp_types[attack_event.player_num_victim - 1]->general_sound_id = GeneralSoundID::HIT_SOUND;
		
		player_sound_comp_types[attack_event.player_num_attacker - 1]->sound_type = SoundType::CHAR_SOUND;
		player_sound_comp_types[attack_event.player_num_attacker - 1]->char_sound_id = CharSoundID::HIT_SOUND;
	}

}


static const float knockback_factor = 90; //pixels per frame

void AttackPowerMechanicSystem::ReactToCollisions(float& dt)
{
	for(auto const& entity: mEntities)
	{
		
		auto& rigidBody = gCoordinator.GetComponent<RigidBody2D>(entity);
		auto& player = gCoordinator.GetComponent<Player>(entity);
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
			player.state = PlayerState::HURTING;
			gen_entity_state.actor_state = EntityState::HURTING_KNOCKBACK;
		}
		else
		{
			if(player.state == PlayerState::HURTING)
			{
				//knock back
				rigidBody.velocity.x = knockback_factor*player_knockback[player.player_num - 1]->x;
				rigidBody.velocity.y = knockback_factor*player_knockback[player.player_num - 1]->y;
				
				gen_entity_state.hurt_anim_time_count += dt;
			
				//if 0.5 seconds seconds have passed, stop hurt animation
				if(gen_entity_state.hurt_anim_time_count >= 0.5f)
				{
					animation.hurt = false;
					gen_entity_state.hurt_anim_time_count = 0;
					player.state = PlayerState::IDLE;
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
		
		auto& player = gCoordinator.GetComponent<Player>(entity);
		
		if(player.attack_box.active && debugRenderAttackBox)
		{
			DrawRectangleRec(player.attack_box.collisionBox, RED);
		}
		
		
		
		if(debugDrawPowerRequest)
		{
			std::string req_num_str = std::to_string(player.requested_power);
			DrawText(req_num_str.c_str(), 100 + 20*player.player_num, 20, 20, RED); 
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
		
		auto& player = gCoordinator.GetComponent<Player>(entity);
		auto& phy_type_comp = gCoordinator.GetComponent<PhysicsTypeComponent>(entity);
		
		float obj_x, obj_y, obj_width, obj_height;
		
		//use attack box for tile position if player is attacking
		if(player.state == PlayerState::ATTACKING)
		{
			obj_x = player.attack_box.collisionBox.x;
			obj_y = player.attack_box.collisionBox.y;
			obj_width = player.attack_box.collisionBox.width;
			obj_height = player.attack_box.collisionBox.height;
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

		}
	}
	
	
}
