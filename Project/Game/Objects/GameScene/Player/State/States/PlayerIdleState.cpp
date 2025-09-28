#include "PlayerIdleState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/PostProcessSystem.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

//============================================================================
//	PlayerIdleState classMethods
//============================================================================

PlayerIdleState::PlayerIdleState() {

	// 初期化値
	targetRadialBlur_.center = Vector2(0.5f, 0.5f);
	targetRadialBlur_.numSamples = 0;
	targetRadialBlur_.width = 0.0f;
}

void PlayerIdleState::Enter(Player& player) {

	canExit_ = false;
	player.SetNextAnimation("player_idle", true, nextAnimDuration_);
}

void PlayerIdleState::Update([[maybe_unused]] Player& player) {

	canExit_ = true;
}

void PlayerIdleState::UpdateAlways([[maybe_unused]] Player& player) {

	// 前状態が切り替えでブラーがかかった状態なら元に戻す処理を行う
	if (preState_ != PlayerState::SwitchAlly) {
		return;
	}

	PostProcessSystem* postProcess = PostProcessSystem::GetInstance();

	// 補間終了
	if (blurTime_ <= blurTimer_) {

		// 初期化値を設定
		postProcess->SetParameter(targetRadialBlur_, PostProcessType::RadialBlur);
		canExit_ = true;
		return;
	} else {

		canExit_ = false;
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

void PlayerIdleState::Exit([[maybe_unused]] Player& player) {

	// リセット
	blurTimer_ = 0.0f;
	canExit_ = true;
}

void PlayerIdleState::ImGui([[maybe_unused]] const Player& player) {

	ImGui::Text("blurTimer: %.3f", blurTimer_);
	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("blurTime", &blurTime_, 0.01f);
	Easing::SelectEasingType(blurEasingType_);
}

void PlayerIdleState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	blurTime_ = JsonAdapter::GetValue<float>(data, "blurTime_");
	blurEasingType_ = static_cast<EasingType>(
		JsonAdapter::GetValue<int>(data, "blurEasingType_"));
}

void PlayerIdleState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["blurTime_"] = blurTime_;
	data["blurEasingType_"] = static_cast<int>(blurEasingType_);
}