#include "BossEnemyGreatAttackIState.h"

//============================================================================
//	BossEnemyGreatAttackIState classMethods
//============================================================================

void BossEnemyGreatAttackIState::Init(BossEnemy* bossEnemy,
	Player* player, FollowCamera* followCamera) {

	bossEnemy_ = nullptr;
	bossEnemy_ = bossEnemy;

	player_ = nullptr;
	player_ = player;

	followCamera_ = nullptr;
	followCamera_ = followCamera;
}