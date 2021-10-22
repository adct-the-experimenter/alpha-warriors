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
		
		
		switch(inputReactor.actor_type)
		{
			case InputReactorType::NONE:{break;}
			case InputReactorType::PLAYER:
			{
				//do player specific event handling
				//get other player specific component from entity.
				
				size_t i = player.player_num - 1;
				//if player not in hurting state
				if(player.state != PlayerState::HURTING)
				{
					//if player's body is in flying state
					if(rigidBody.in_flying_state)
					{
						//if moved left joystick up
						if(input.gamepads_vec[i].left_y_dir_axis == -1 
							|| input.gamepads_vec[i].left_y_axis < -0.5*joystick_border)
						{
							rigidBody.velocity.y = -speed_factor*player.speed_factor;
						}
						//else if move left joystick down
						else if(input.gamepads_vec[i].left_y_dir_axis == 1 
							|| input.gamepads_vec[i].left_y_axis > 0.5*joystick_border)
						{
							rigidBody.velocity.y = speed_factor*player.speed_factor;
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
						rigidBody.velocity.x = -speed_factor*player.speed_factor;
					}
					//else if moved left joystick right
					else if( input.gamepads_vec[i].left_x_dir_axis == 1 
							|| input.gamepads_vec[i].left_x_axis > 0.5*joystick_border )
					{
						rigidBody.velocity.x = speed_factor*player.speed_factor;
					}
					else
					{
						rigidBody.velocity.x = 0.0f;
					}
					
					
					//if jump button pressed
					if( input.gamepads_vec[i].button_down == SDL_CONTROLLER_BUTTON_B)
					{
						rigidBody.jump_speed = -player.jump_factor;
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
					if(input.gamepads_vec[i].button_up_released == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)
					{
						player.powerButtonPressed = true;
					}
					//if regular attack button pressed
					if(input.gamepads_vec[i].button_up_released == SDL_CONTROLLER_BUTTON_A)
					{
						player.regularAttackButtonPressed = true;
					}
					//if craft button pressed, any of the trigger buttons
					if(input.gamepads_vec[i].button_up_released == SDL_CONTROLLER_BUTTON_X)
					{
						player.craftButtonPressed = true;
					}
					//if energy beam button pressed 
					if(input.gamepads_vec[i].button_up_released == SDL_CONTROLLER_BUTTON_Y)
					{
						player.energyButtonPressed = true;
					}
					
				}				
				
				break;
			}
			case InputReactorType::CAR:{break;}
			default:{break;}
		}
		
	}
}