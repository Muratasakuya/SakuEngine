#include "PlayerIState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Core/PostProcessSystem.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

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