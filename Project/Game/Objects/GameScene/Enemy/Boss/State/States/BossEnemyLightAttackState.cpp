#include "BossEnemyLightAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Effect/Game/Helper/GameEffectCommandHelper.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyLightAttackState classMethods
//============================================================================

void BossEnemyLightAttackState::Enter(BossEnemy& bossEnemy) {

	// 攻撃予兆アニメーションを設定
	bossEnemy.SetNextAnimation("bossEnemy_lightAttackParrySign", false, nextAnimDuration_);

	// 座標を設定
	startPos_ = bossEnemy.GetTranslation();
	canExit_ = false;

	// 攻撃予兆を出す
	Vector3 sign = bossEnemy.GetTranslation();
	sign.y = 2.0f;
	attackSign_->Emit(ProjectToScreen(sign, *followCamera_));

	// パリィ可能にする
	bossEnemy.ResetParryTiming();
	parryParam_.continuousCount = 1;
	parryParam_.canParry = true;

	parried_ = false;
}

void BossEnemyLightAttackState::Update(BossEnemy& bossEnemy) {

	// パリィ攻撃のタイミングを更新
	UpdateParryTiming(bossEnemy);

	// 状態に応じて更新
	switch (currentState_) {
	case BossEnemyLightAttackState::State::ParrySign: {

		// プレイヤーの方を向きながら補間
		UpdateParrySign(bossEnemy);
		break;
	}
	case BossEnemyLightAttackState::State::Attack: {

		// 攻撃、終了後状態を終了
		UpdateAttack(bossEnemy);
		break;
	}
	}
}

void BossEnemyLightAttackState::UpdateParrySign(BossEnemy& bossEnemy) {

	// アニメーション終了時間で補間
	lerpTimer_ += GameTimer::GetScaledDeltaTime();
	float lerpT = std::clamp(lerpTimer_ / bossEnemy.GetAnimationDuration(
		"bossEnemy_lightAttackParrySign"), 0.0f, 1.0f);
	lerpT = EasedValue(easingType_, lerpT);

	// 目標座標を常に更新する
	const Vector3 playerPos = player_->GetTranslation();
	Vector3 direction = (bossEnemy.GetTranslation() - playerPos).Normalize();
	Vector3 target = playerPos - direction * attackOffsetTranslation_;
	target.y = 0.0f;
	LookTarget(bossEnemy, playerPos);

	// 座標補間
	bossEnemy.SetTranslation(Vector3::Lerp(startPos_, target, lerpT));

	// アニメーションが終了次第攻撃する
	if (bossEnemy.IsAnimationFinished()) {

		bossEnemy.SetTranslation(target);
		bossEnemy.SetNextAnimation("bossEnemy_lightAttack", false, nextAnimDuration_);

		// 状態を進める
		currentState_ = State::Attack;
	}
}

void BossEnemyLightAttackState::UpdateAttack(BossEnemy& bossEnemy) {

	// animationが終了したら経過時間を進める
	if (bossEnemy.IsAnimationFinished()) {

		exitTimer_ += GameTimer::GetDeltaTime();
		// 時間経過が過ぎたら遷移可能
		if (exitTime_ < exitTimer_) {

			canExit_ = true;
		}
	}
}


void BossEnemyLightAttackState::UpdateParryTiming(BossEnemy& bossEnemy) {

	// パリィ攻撃のタイミング
	switch (currentState_) {
	case BossEnemyLightAttackState::State::Attack: {

		if (bossEnemy.IsEventKey("Parry", 0)) {

			bossEnemy.TellParryTiming();
			parried_ = true;
		}
		break;
	}
	}
}

void BossEnemyLightAttackState::Exit([[maybe_unused]] BossEnemy& bossEnemy) {

	// リセット
	canExit_ = false;
	parryParam_.canParry = false;
	lerpTimer_ = 0.0f;
	exitTimer_ = 0.0f;
	currentState_ = State::ParrySign;
	bossEnemy.ResetParryTiming();
}

void BossEnemyLightAttackState::ImGui(const BossEnemy& bossEnemy) {

	ImGui::Text(std::format("parried: {}", parried_).c_str());
	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);

	ImGui::DragFloat("attackOffsetTranslation", &attackOffsetTranslation_, 0.1f);
	ImGui::DragFloat("exitTime", &exitTime_, 0.01f);

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());
	ImGui::Text("exitTimer: %.3f", exitTimer_);
	Easing::SelectEasingType(easingType_);

	// 座標を設定
	Vector3 start = bossEnemy.GetTranslation();
	const Vector3 playerPos = player_->GetTranslation();
	Vector3 direction = (start - playerPos).Normalize();
	Vector3 target = playerPos - direction * attackOffsetTranslation_;
	target.y = 2.0f;
	LineRenderer::GetInstance()->DrawSphere(8, 2.0f, target, Color::Red());
}

void BossEnemyLightAttackState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = JsonAdapter::GetValue<float>(data, "rotationLerpRate_");

	attackOffsetTranslation_ = JsonAdapter::GetValue<float>(data, "attackOffsetTranslation_");
	exitTime_ = JsonAdapter::GetValue<float>(data, "exitTime_");
	easingType_ = static_cast<EasingType>(JsonAdapter::GetValue<int>(data, "easingType_"));
}

void BossEnemyLightAttackState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;

	data["attackOffsetTranslation_"] = attackOffsetTranslation_;
	data["exitTime_"] = exitTime_;
	data["easingType_"] = static_cast<int>(easingType_);
}