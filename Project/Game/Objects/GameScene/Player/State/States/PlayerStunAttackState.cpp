#include "PlayerStunAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Core/PostProcessSystem.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

//============================================================================
//	PlayerStunAttackState classMethods
//============================================================================

PlayerStunAttackState::PlayerStunAttackState(GameObject3D* ally) {

	// 味方を設定
	ally_ = nullptr;
	ally_ = ally;

	// 初期化値
	targetRadialBlur_.center = Vector2(0.5f, 0.5f);
	targetRadialBlur_.numSamples = 0;
	targetRadialBlur_.width = 0.0f;

	currentState_ = State::AllyEntry;
	canExit_ = false;
}

void PlayerStunAttackState::Enter([[maybe_unused]] Player& player) {

	// 座標取得
	const Vector3 enemyPos = bossEnemy_->GetTranslation();

	// 敵との距離分離した位置を補間開始座標とする
	Vector3 toCamera = followCamera_->GetTransform().translation - enemyPos;
	toCamera.y = 0.0f;
	toCamera = toCamera.Normalize();
	rushStartAllyTranslation_ = enemyPos + toCamera * enemyDistance_;
	rushStartAllyTranslation_.y = targetTranslationY_;
	// 目標座標は敵の座標
	rushTargetAllyTranslation_ = enemyPos;
	rushTargetAllyTranslation_.y = targetTranslationY_;

	// カメラの状態を味方攻撃に切り替える
	followCamera_->SetState(FollowCameraState::AllyAttack);
}

void PlayerStunAttackState::Update(Player& player) {

	// ブラーの状態を元に戻す
	UpdateBlur();

	// 状態に応じて更新
	switch (currentState_) {
	case PlayerStunAttackState::State::AllyEntry: {

		UpdateAllyEntry(player);
		break;
	}
	case PlayerStunAttackState::State::AllyRushAttack: {

		UpdateAllyRushAttack(player);
		break;
	}
	case PlayerStunAttackState::State::PlayerAttack: {

		UpdatePlayerAttack(player);
		break;
	}
	}
}

void PlayerStunAttackState::UpdateBlur() {

	PostProcessSystem* postProcess = PostProcessSystem::GetInstance();

	// 補間終了
	if (blurTime_ <= blurTimer_) {

		// 初期化値を設定
		postProcess->SetParameter(targetRadialBlur_, PostProcessType::RadialBlur);
		return;
	}

	// ブラー更新
	blurTimer_ += GameTimer::GetDeltaTime();

	float lerpT = std::clamp(blurTimer_ / blurTime_, 0.0f, 1.0f);
	lerpT = EasedValue(blurEasingType_, lerpT);

	// 各値を0へ補間(ブラーを消す)
	radialBlur_.center.y = std::lerp(startRadialBlur_.center.y, 0.5f, lerpT);
	radialBlur_.numSamples = static_cast<int>(std::round(std::lerp(
		static_cast<float>(startRadialBlur_.numSamples), 0.0f, lerpT)));
	radialBlur_.width = std::lerp(startRadialBlur_.width, 0.0f, lerpT);

	// 値を設定
	postProcess->SetParameter(radialBlur_, PostProcessType::RadialBlur);
}

void PlayerStunAttackState::UpdateAllyEntry(Player& player) {

	// 時間経過を進める
	entryTimer_ += GameTimer::GetDeltaTime();
	float lerpT = entryTimer_ / entryTime_;
	lerpT = EasedValue(entryEasingType_, lerpT);

	// 座標を設定
	Vector3 allyTranslation = ally_->GetTranslation();
	// Y座標を補間
	allyTranslation.y = std::lerp(0.0f, targetTranslationY_, lerpT);
	ally_->SetTranslation(allyTranslation);

	// playerの表示を消す(とりあえずα値を下げる)
	player.SetAlpha(std::clamp(1.0f - lerpT, 0.0f, 1.0f));

	if (1.0f <= lerpT) {

		// 次の状態に遷移させる
		currentState_ = State::AllyRushAttack;
		// このtimerは再利用するためリセット
		entryTimer_ = 0.0f;
	}
}

void PlayerStunAttackState::UpdateAllyRushAttack(Player& player) {

	// 時間経過を進める
	rushTimer_ += GameTimer::GetDeltaTime();
	float lerpT = rushTimer_ / rushTime_;
	lerpT = EasedValue(rushEasingType_, lerpT);

	// 座標を補間して設定
	Vector3 translation = Vector3::Lerp(rushStartAllyTranslation_,
		rushTargetAllyTranslation_, lerpT);
	ally_->SetTranslation(translation);

	if (1.0f <= lerpT) {

		// 次の状態に遷移させる
		currentState_ = State::PlayerAttack;

		// この瞬間に次のanimationに切り替える
		player.SetNextAnimation("player_stunAttack", false, nextAnimDuration_);

		// 一瞬スローモーションにするためにdeltaTimeをスケーリングさせる
		GameTimer::SetTimeScale(0.0f);

		// カメラの状態をプレイヤー攻撃に切り替える
		followCamera_->SetState(FollowCameraState::StunAttack);
	}
}

void PlayerStunAttackState::UpdatePlayerAttack(Player& player) {

	// 時間経過を進める
	entryTimer_ += GameTimer::GetDeltaTime();
	float lerpT = entryTimer_ / entryTime_;
	lerpT = EasedValue(entryEasingType_, lerpT);

	// playerを出現させる
	player.SetAlpha(std::clamp(lerpT, 0.0f, 1.0f));

	// animationが終了次第遷移可能状態にする
	if (player.IsAnimationFinished()) {

		canExit_ = true;

		// カメラの状態を元の操作状態に戻す
		followCamera_->SetState(FollowCameraState::Follow);

		// 画面シェイクを行わせる
		followCamera_->SetScreenShake(true);
	}
}

void PlayerStunAttackState::Exit([[maybe_unused]] Player& player) {

	// リセット
	currentState_ = State::AllyEntry;
	entryTimer_ = 0.0f;
	rushTimer_ = 0.0f;
	blurTimer_ = 0.0f;
	canExit_ = false;
}

void PlayerStunAttackState::ImGui([[maybe_unused]] const Player& player) {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.01f);
	ImGui::DragFloat("blurTime", &blurTime_, 0.01f);
	Easing::SelectEasingType(blurEasingType_);
	ImGui::DragFloat("entryTime", &entryTime_, 0.01f);
	ImGui::DragFloat("targetTranslationY", &targetTranslationY_, 0.01f);
	ImGui::DragFloat("enemyDistance", &enemyDistance_, 0.01f);
	Easing::SelectEasingType(entryEasingType_, "entryEasingType");
	ImGui::DragFloat("rushTime", &rushTime_, 0.01f);
	Easing::SelectEasingType(rushEasingType_, "rushEasingType_");
}

void PlayerStunAttackState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	entryTime_ = JsonAdapter::GetValue<float>(data, "entryTime_");
	targetTranslationY_ = JsonAdapter::GetValue<float>(data, "targetTranslationY_");
	enemyDistance_ = JsonAdapter::GetValue<float>(data, "enemyDistance_");
	rushTime_ = JsonAdapter::GetValue<float>(data, "rushTime_");
	blurTime_ = JsonAdapter::GetValue<float>(data, "blurTime_");
	entryEasingType_ = static_cast<EasingType>(
		JsonAdapter::GetValue<int>(data, "entryEasingType_"));
	rushEasingType_ = static_cast<EasingType>(
		JsonAdapter::GetValue<int>(data, "rushEasingType_"));
	blurEasingType_ = static_cast<EasingType>(
		JsonAdapter::GetValue<int>(data, "blurEasingType_"));
}

void PlayerStunAttackState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["entryTime_"] = entryTime_;
	data["targetTranslationY_"] = targetTranslationY_;
	data["enemyDistance_"] = enemyDistance_;
	data["rushTime_"] = rushTime_;
	data["blurTime_"] = blurTime_;
	data["entryEasingType_"] = static_cast<int>(entryEasingType_);
	data["rushEasingType_"] = static_cast<int>(rushEasingType_);
	data["blurEasingType_"] = static_cast<int>(blurEasingType_);
}