#include "InputReactorSystem.h"

#include "core/system.h"
#include "core/coordinator.h"


extern Coordinator gCoordinator;

float speed_factor = 200.0f;

static const int16_t joystick_border = 32600;
static const int16_t joystick_border_analog = 10000;


void InputReactorSystem::Update(ControllerInput& input)
{
	for (auto const& entity : mEntities)
	{
		auto& inputReactor = gCoordinator.GetComponent<InputReact>(entity);
		auto& rigidBody = gCoordinator.GetComponent<RigidBody2D>(entity);
		auto& player = gCoordinator.GetComponent<Player>(entity);
		auto& gen_entity_state = gCoordinator.GetComponent<GeneralEnityState>(entity);
		
		//skip player input if not alive
		if(!gen_entity_state.alive){continue;}
		
		switch(inputReactor.actor_type)
		{
			case InputReactorType::NONE:{break;}
			case InputReactorType::PLAYER:
			{
				//do player specific event handling
				//get other player specific component from entity.
				
				size_t i = player.player_num - 1;
				//if player not in hurting state
				if(gen_entity_state.actor_state != EntityState::HURTING_KNOCKBACK)
				{
					//if player's body is in flying state
					if(rigidBody.in_flying_state)
					{
						//if moved left joystick up
						if(input.gamepads_vec[i].left_y_dir_axis == -1 
							|| input.gamepads_vec[i].left_y_axis < -0.5*joystick_border)
						{
							rigidBody.velocity.y = -speed_factor*gen_entity_state.speed_factor;
						}
						//else if move left joystick down
						else if(input.gamepads_vec[i].left_y_dir_axis == 1 
							|| input.gamepads_vec[i].left_y_axis > 0.5*joystick_border)
						{
							rigidBody.velocity.y = speed_factor*gen_entity_state.speed_factor;
						}
						else
						{
							rigidBody.velocity.y = 0.0f;
						}
					}
					//if player is not in flying state and not falling
					else if(!rigidBody.in_flying_state && !rigidBody.velocity.y)
					{
						//if moved left joystick up
						if(input.gamepads_vec[i].left_y_dir_axis == -1 
							|| input.gamepads_vec[i].left_y_axis < -0.5*joystick_border)
						{
							rigidBody.velocity.y = -0.1f;
						}
						//else if move left joystick down
						else if(input.gamepads_vec[i].left_y_dir_axis == 1 
							|| input.gamepads_vec[i].left_y_axis > 0.5*joystick_border)
						{
							rigidBody.velocity.y = 0.1f;
						}
						else
						{
							rigidBody.velocity.y = 0.0f;
						}
					}
					
					
					//if moved left joystick left
					if(input.gamepads_vec[i].left_x_dir_axis == -1 
						|| input.gamepads_vec[i].left_x_axis < -0.5*joystick_border )
					{
						rigidBody.velocity.x = -speed_factor*gen_entity_state.speed_factor;
					}
					//else if moved left joystick right
					else if( input.gamepads_vec[i].left_x_dir_axis == 1 
							|| input.gamepads_vec[i].left_x_axis > 0.5*joystick_border )
					{
						rigidBody.velocity.x = speed_factor*gen_entity_state.speed_factor;
					}
					else
					{
						rigidBody.velocity.x = 0.0f;
					}
					
					
					//if jump button pressed
					if( input.gamepads_vec[i].button_down == SDL_CONTROLLER_BUTTON_B && !gen_entity_state.inTeleportMode)
					{
						rigidBody.jump_speed = -gen_entity_state.jump_factor;
					}
					else
					{
						rigidBody.jump_speed = 0;
					}
					
					//if fly mode activate/deactive button pressed
					if(input.gamepads_vec[i].button_up_released == SDL_CONTROLLER_BUTTON_LEFTSHOULDER)
					{
						rigidBody.in_flying_state = !rigidBody.in_flying_state;
					}
					
					//if special power button pressed
					if(input.gamepads_vec[i].button_up_released == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER
						&& !gen_entity_state.inTeleportMode)
					{
						gen_entity_state.powerButtonPressed = true;
					}
					//if regular attack button is released
					if(input.gamepads_vec[i].button_up_released == SDL_CONTROLLER_BUTTON_A
						&& !gen_entity_state.inTeleportMode)
					{
						gen_entity_state.regularAttackButtonPressed = true;
						gen_entity_state.regularAttackButtonHeld = false;
						gen_entity_state.time_reg_attack_button_held = 0.0f;
						
					}
					else
					{
						gen_entity_state.regularAttackButtonPressed = false;
					}
					
					//if regular attack button held down
					if(input.gamepads_vec[i].button_held_array[SDL_CONTROLLER_BUTTON_A]
						&& !gen_entity_state.inTeleportMode && input.gamepads_vec[i].button_up_released != SDL_CONTROLLER_BUTTON_A)
					{
						gen_entity_state.regularAttackButtonHeld = true;
					}
					else
					{
						gen_entity_state.regularAttackButtonHeld = false;
					}
					
					//if craft button pressed, any of the trigger buttons
					if(input.gamepads_vec[i].button_up_released == SDL_CONTROLLER_BUTTON_X
						&& !gen_entity_state.inTeleportMode)
					{
						gen_entity_state.craftButtonPressed = true;
					}
					
					//if energy beam button released 
					if(input.gamepads_vec[i].button_up_released == SDL_CONTROLLER_BUTTON_Y
						&& !gen_entity_state.inTeleportMode)
					{
						gen_entity_state.energyButtonPressed = true;
						gen_entity_state.time_energy_button_held = 0.0f;
						gen_entity_state.energyButtonHeld = false;
					}
					else
					{
						gen_entity_state.energyButtonPressed = false;
					}
					
					//if energy button is held down
					if(input.gamepads_vec[i].button_held_array[SDL_CONTROLLER_BUTTON_Y]
						&& !gen_entity_state.inTeleportMode && input.gamepads_vec[i].button_up_released != SDL_CONTROLLER_BUTTON_Y)
					{
						gen_entity_state.energyButtonHeld = true;
					}
					else
					{
						gen_entity_state.energyButtonHeld = false;
					}
					
					//if teleport mode activation requested with button combination
					if(input.gamepads_vec[i].button_held_array[SDL_CONTROLLER_BUTTON_B] &&
					   input.gamepads_vec[i].button_held_array[SDL_CONTROLLER_BUTTON_A])
					{
						gen_entity_state.teleportButton = true;
						gen_entity_state.time_energy_button_held = 0.0f;
						gen_entity_state.energyButtonHeld = false;
						gen_entity_state.energyButtonPressed = false;
						gen_entity_state.regularAttackButtonPressed = false;
					}
					else
					{
						gen_entity_state.teleportButton = false;
					}
				}				
				
				break;
			}
			case InputReactorType::CAR:{break;}
			default:{break;}
		}
		
	}
}
