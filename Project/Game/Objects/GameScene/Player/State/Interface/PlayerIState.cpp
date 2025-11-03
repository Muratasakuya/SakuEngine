#include "PlayerIState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Core/PostProcessSystem.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	PlayerIState classMethods
//============================================================================

PlayerIState::PlayerIState() {

	postProcess_ = nullptr;
	postProcess_ = PostProcessSystem::GetInstance();
}

void PlayerIState::SetRotateToDirection(Player& player, const Vector3& move) {

	Vector3 direction = Vector3(move.x, 0.0f, move.z).Normalize();

	if (direction.Length() <= epsilon_) {
		return;
	}

	// 向きを計算
	Quaternion targetRotation = Quaternion::LookRotation(direction, Vector3(0.0f, 1.0f, 0.0f));
	Quaternion rotation = player.GetRotation();
	rotation = Quaternion::Slerp(rotation, targetRotation, rotationLerpRate_);
	player.SetRotation(rotation);
}

Vector3 PlayerIState::GetPlayerFixedYPos() const {

	Vector3 translation = player_->GetTranslation();
	// Y座標を固定
	translation.y = 0.0f;

	return translation;
}

Vector3 PlayerIState::GetBossEnemyFixedYPos() const {

	Vector3 translation = bossEnemy_->GetTranslation();
	// Y座標を固定
	translation.y = 0.0f;

	return translation;
}

float PlayerIState::GetDistanceToBossEnemy() const {

	// プレイヤーとボス敵の距離を取得
	float distance = Vector3(GetBossEnemyFixedYPos() - GetPlayerFixedYPos()).Length();
	return distance;
}

Vector3 PlayerIState::GetDirectionToBossEnemy() const {

	// プレイヤーからボス敵への方向を取得
	Vector3 direction = Vector3(GetBossEnemyFixedYPos() - GetPlayerFixedYPos()).Normalize();
	return direction;
}