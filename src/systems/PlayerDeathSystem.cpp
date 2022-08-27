#include "PlayerDeathSystem.h"

#include "core/system.h"

#include "core/coordinator.h"

#include <iostream>

#include <cmath>

extern Coordinator gCoordinator;

#include "misc/level_maps.h"

#define HALF_GAME_SCREEN_WIDTH 320

#define HEALTH_BAR_HEIGHT 40

//assuming base health is 30 and max health factor is 1.5, 45 is max health
#define MAX_HEALTH 45
#define HEALTH_BAR_PIXELS HALF_GAME_SCREEN_WIDTH / MAX_HEALTH

static Color HealthBarColor = (Color){100,200,100,250};


void PlayerDeathSystem::Init(std::uint8_t num_players)
{
	winning_player = -1;
	
	players_alive.reset();
	for(std::uint8_t i = 0; i < num_players; i++)
	{
		players_alive[i] = 1;
	}
}

static int16_t player_vs_health_array[2] = {0,0};

void PlayerDeathSystem::RenderHealthBars_2PlayerFight()
{
	//get info on player health
	for (auto const& entity : mEntities)
	{
		auto& player = gCoordinator.GetComponent<Player>(entity);
		auto& gen_entity_state = gCoordinator.GetComponent<GeneralEnityState>(entity);
		
		player_vs_health_array[player.player_num - 1] = gen_entity_state.health;
	}
	
	
	
	//render player 1 health bar on the left side of the screen
	DrawRectangle(0, 0, player_vs_health_array[0]*HEALTH_BAR_PIXELS, HEALTH_BAR_HEIGHT, HealthBarColor);
	
	//render player 2 health bar on the right side of the screen
	DrawRectangle(HALF_GAME_SCREEN_WIDTH + (MAX_HEALTH - player_vs_health_array[1])*HEALTH_BAR_PIXELS, 0, player_vs_health_array[1]*HEALTH_BAR_PIXELS, HEALTH_BAR_HEIGHT, HealthBarColor);
}

void PlayerDeathSystem::Update()
{
	
	if(!one_player_won)
	{
		//for every entity
		for (auto const& entity : mEntities)
		{
			auto& render_comp = gCoordinator.GetComponent<RenderModelComponent>(entity);
			auto& player = gCoordinator.GetComponent<Player>(entity);
			auto& gen_entity_state = gCoordinator.GetComponent<GeneralEnityState>(entity);
			
			
			if(gen_entity_state.health <= 0)
			{
				gen_entity_state.alive = false;
				gen_entity_state.actor_state = EntityState::DEAD;
				gen_entity_state.health = 0;
			}
			
			if(!gen_entity_state.alive && render_comp.render)
			{
				players_alive[player.player_num - 1] = 0;
				render_comp.render = false;
			}
			
		}
		
		//check if only 1 player is alive
		if(players_alive.count() == 1 && winning_player == -1)
		{		
			one_player_won = true;
			
			//set winning player
			for(std::uint8_t i = 0; i < 8; i++)
			{
				if(players_alive[i] == 1)
				{
					winning_player = i;
				}
			}
		}
	}
	
	 
	
}

bool PlayerDeathSystem::OnePlayerWon(){return one_player_won;}
	
std::int8_t PlayerDeathSystem::GetPlayerWhoWon(){return winning_player;}

void PlayerDeathSystem::Reset()
{
	winning_player = -1;
	one_player_won = false;
}
