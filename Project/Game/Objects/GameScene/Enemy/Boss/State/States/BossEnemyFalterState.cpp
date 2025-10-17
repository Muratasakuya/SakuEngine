#include "BossEnemyFalterState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyFalterState classMethods
//============================================================================

void BossEnemyFalterState::Enter(BossEnemy& bossEnemy) {

	bossEnemy.SetNextAnimation("bossEnemy_falter", false, nextAnimDuration_);
}

void BossEnemyFalterState::Update(BossEnemy& bossEnemy) {

	const float deltaTime = GameTimer::GetScaledDeltaTime();

	// 前方ベクトルを取得
	Vector3 bossPos = bossEnemy.GetTranslation();
	Vector3 playerPos = player_->GetTransform().translation;

	// 回転を計算して設定
	Quaternion bossRotation = Quaternion::LookTarget(bossPos, playerPos,
		Vector3(0.0f, 1.0f, 0.0f), bossEnemy.GetRotation(), rotationLerpRate_ * deltaTime);
	bossEnemy.SetRotation(bossRotation);

	// 後ずさりさせる
	Vector3 backStepVelocity = bossEnemy.GetTransform().GetBack() * backStepSpeed_ * deltaTime;
	bossEnemy.SetTranslation(bossPos + backStepVelocity);

	// アニメーションが終了次第状態を終了
	if (bossEnemy.IsAnimationFinished()) {

		canExit_ = true;
	}
}

void BossEnemyFalterState::Exit([[maybe_unused]] BossEnemy& bossEnemy) {
}

void BossEnemyFalterState::ImGui([[maybe_unused]] const BossEnemy& bossEnemy) {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("backStepSpeed", &backStepSpeed_, 0.001f);
}

void BossEnemyFalterState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	backStepSpeed_ = JsonAdapter::GetValue<float>(data, "backStepSpeed_");
}

void BossEnemyFalterState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["backStepSpeed_"] = backStepSpeed_;
}