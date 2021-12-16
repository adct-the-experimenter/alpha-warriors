
#ifndef ATTACK_POWER_MECHANIC_SYSTEM_H
#define ATTACK_POWER_MECHANIC_SYSTEM_H

#include "core/system.h"

#include <queue>

#include "misc/level_maps.h"
#include "misc/game-constants.h"

struct PowerTransferTransaction
{
	std::uint8_t receivingPlayer = 0;
	std::bitset <8> powerTransfered = 0;
};

struct AttackEvent
{
	bool attack = false;
	std::uint8_t player_num_victim = 0;
	std::uint8_t player_num_attacker = 0;
	
};

class AttackPowerMechanicSystem : public System
{
public:
	void Init(std::uint8_t num_players);
	
	void HandlePowerActivation(float& dt);
	
	void MoveAttackBoxesWithPlayer(float& dt);
	
	//for checking collision between attack box  and player
	void CollisionDetectionBetweenPlayers();
	
	//for setting player reaction to attack
	void ReactToCollisions(float& dt);
	
	//for destruction of tiles
	void HandleCollisionBetweenPlayerAttacksAndWorldTiles();
	
	void DebugRender();
	
private:
	std::queue <PowerTransferTransaction> power_transfer_transaction_queue;
	
	//pointers for use with collision detection and health management
	std::array <AttackBox*,MAX_NUMBER_OF_ENTITIES_IN_GAME> entity_attack_boxes_ptrs;
	std::array <AttackBox*,MAX_NUMBER_OF_ENTITIES_IN_GAME> entity_special_attack_boxes_ptrs;
	std::array <std::int16_t*,MAX_NUMBER_OF_ENTITIES_IN_GAME> entity_health_ptrs;
	std::array <Vector2*,MAX_NUMBER_OF_ENTITIES_IN_GAME> entity_position_ptrs;
	std::array <std::uint8_t*,MAX_NUMBER_OF_ENTITIES_IN_GAME> entity_last_hit_by_ptrs;
	std::array <bool*,MAX_NUMBER_OF_ENTITIES_IN_GAME> entity_alive_ptrs;
	std::array <bool*,MAX_NUMBER_OF_ENTITIES_IN_GAME> entity_taking_damage_state_ptrs;
	std::array <bool*,MAX_NUMBER_OF_ENTITIES_IN_GAME> entity_hurt_invincible_ptrs;
	std::array <float*,MAX_NUMBER_OF_ENTITIES_IN_GAME> entity_attack_damage_factor_ptrs;
	
	std::array <SoundComponent*,MAX_NUMBER_OF_ENTITIES_IN_GAME> entity_sound_comp_types;
	
	//collision handling functions to determine if players hit each other
	bool AreBothPlayersAlive(int& player_a_num, int& player_b_num);
	AttackEvent CheckCollisionBetween2Players(int& player_a_num, int& player_b_num);
	void HandlePossibleCollisionBetweenPlayers(int& player_a_num, int& player_b_num);
	
	//array to hold values for knockback of player who was hit
	std::array <Vector2*,MAX_NUMBER_OF_ENTITIES_IN_GAME> entity_knockback;
	
	std::uint8_t m_num_players;
};

#endif
