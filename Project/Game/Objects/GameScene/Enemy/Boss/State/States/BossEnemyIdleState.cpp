#include "BossEnemyIdleState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyIdleState classMethods
//============================================================================

void BossEnemyIdleState::Enter(BossEnemy& bossEnemy) {

	canExit_ = false;
	// animationをリセットする
	bossEnemy.ResetAnimation();

	bossEnemy.SetNextAnimation("bossEnemy_idle", true, nextAnimDuration_);
}

void BossEnemyIdleState::Update(BossEnemy& bossEnemy) {

	// playerの方を向かせる
	LookTarget(bossEnemy, player_->GetTranslation());

	// 後ずさりさせる
	Vector3 bossPos = bossEnemy.GetTranslation();
	Vector3 backStepVelocity = bossEnemy.GetTransform().GetBack() * backStepSpeed_ * GameTimer::GetScaledDeltaTime();
	bossEnemy.SetTranslation(bossPos + backStepVelocity);

	// animationが終了したら遷移可能にする
	if (bossEnemy.GetAnimationRepeatCount() != 0) {

		canExit_ = true;
	}
}

void BossEnemyIdleState::Exit(BossEnemy& bossEnemy) {

	canExit_ = false;
	// animationをリセットする
	bossEnemy.ResetAnimation();
}

void BossEnemyIdleState::ImGui([[maybe_unused]] const BossEnemy& bossEnemy) {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("backStepSpeed", &backStepSpeed_, 0.001f);
}

void BossEnemyIdleState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	backStepSpeed_ = JsonAdapter::GetValue<float>(data, "backStepSpeed_");
}

void BossEnemyIdleState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["backStepSpeed_"] = backStepSpeed_;
}