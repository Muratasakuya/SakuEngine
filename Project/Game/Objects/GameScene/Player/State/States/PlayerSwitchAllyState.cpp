#include "PlayerSwitchAllyState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/PostProcessSystem.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Engine/Utility/Json/JsonAdapter.h>

// imgui
#include <imgui.h>

//============================================================================
//	PlayerSwitchAllyState classMethods
//============================================================================

PlayerSwitchAllyState::PlayerSwitchAllyState() {

	// 変更しない値(中心)
	radialBlur_.center.x = 0.5f;
	canExit_ = false;

	// NONEで初期化
	selectState_ = PlayerState::None;
}

void PlayerSwitchAllyState::Enter(Player& player) {

	// Noneで初期化
	selectState_ = PlayerState::None;

	// deltaTimeをスケーリングしても元の値に戻らないようにする
	GameTimer::SetReturnScaleEnable(false);

	// カメラの状態を切り替え待ち状態にする
	followCamera_->SetState(FollowCameraState::SwitchAlly);
	// 画面シェイクを止める
	followCamera_->SetScreenShake(false);

	// HPなどの表示を消してスタン用のHUDを出す
	player.GetHUD()->SetDisable();
	player.GetStunHUD()->SetVaild();
}

void PlayerSwitchAllyState::Update(Player& player) {

	// 選択状態に入れるためにdeltaTimeを0.0f近くまで下げる
	deltaTimeScaleTimer_ += GameTimer::GetScaledDeltaTime();
	float lerpT = deltaTimeScaleTimer_ / deltaTimeScaleTime_;
	lerpT = EasedValue(deltaTimeScaleEasingType_, lerpT);
	float deltaScale = std::lerp(1.0f, deltaTimeScale_, lerpT);
	// スケーリング値を設定
	GameTimer::SetTimeScale(std::clamp(deltaScale, 0.0f, 1.0f));

	// 入力受付待ち
	switchAllyTimer_ += GameTimer::GetDeltaTime();
	float t = switchAllyTimer_ / switchAllyTime_;

	// ブラー更新
	UpdateBlur();

	// 入力状態の確認
	CheckInput(t);

	// 遷移可能(何かを選択した)になったらHUDを元に戻す
	if (canExit_) {

		player.GetHUD()->SetValid();
		player.GetStunHUD()->SetCancel();
	}
}

void PlayerSwitchAllyState::UpdateBlur() {

	PostProcessSystem* postProcess = PostProcessSystem::GetInstance();

	// ブラー更新
	blurTimer_ += GameTimer::GetDeltaTime();

	// 補間終了
	if (blurTime_ <= blurTimer_) {

		postProcess->SetParameter(targetRadialBlur_, PostProcessType::RadialBlur);
		return;
	}
	float lerpT = std::clamp(blurTimer_ / blurTime_, 0.0f, 1.0f);
	lerpT = EasedValue(blurEasingType_, lerpT);

	// 各値を補間
	radialBlur_.center.y = std::lerp(0.5f, targetRadialBlur_.center.y, lerpT);
	radialBlur_.numSamples = static_cast<int>(std::round(std::lerp(
		0.0f, static_cast<float>(targetRadialBlur_.numSamples), lerpT)));
	radialBlur_.width = std::lerp(0.0f, targetRadialBlur_.width, lerpT);

	// 値を設定
	postProcess->SetParameter(radialBlur_, PostProcessType::RadialBlur);
}

void PlayerSwitchAllyState::CheckInput(float t) {

	// 切り替えた
	if (inputMapper_->IsTriggered(PlayerInputAction::Switching)) {

		selectState_ = PlayerState::StunAttack;
		canExit_ = true;
		return;
	}
	// 切り替え無し
	if (inputMapper_->IsTriggered(PlayerInputAction::NotSwitching)) {

		selectState_ = PlayerState::Idle;
		canExit_ = true;

		// カメラの状態を元に戻させる
		followCamera_->SetState(FollowCameraState::Follow);
		return;
	}

	// 入力時間切れ
	if (1.0f < t) {

		// 切り替えなし
		selectState_ = PlayerState::Idle;
		canExit_ = true;

		// カメラの状態を元に戻させる
		followCamera_->SetState(FollowCameraState::Follow);
	}
}

void PlayerSwitchAllyState::Exit([[maybe_unused]] Player& player) {

	// deltaTimeを元の値に戻るようにする
	GameTimer::SetReturnScaleEnable(true);

	// リセット
	deltaTimeScaleTimer_ = 0.0f;
	switchAllyTimer_ = 0.0f;
	blurTimer_ = 0.0f;
	canExit_ = false;
}

void PlayerSwitchAllyState::ImGui([[maybe_unused]] const Player& player) {

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());
	ImGui::DragFloat("deltaTimeScaleTime", &deltaTimeScaleTime_, 0.01f);
	ImGui::DragFloat("switchAllyTime", &switchAllyTime_, 0.01f);
	ImGui::DragFloat("deltaTimeScale", &deltaTimeScale_, 0.01f);
	ImGui::DragFloat("blurTime", &blurTime_, 0.01f);
	ImGui::DragFloat("targetRadialBlurCenterY", &targetRadialBlur_.center.y, 0.01f);
	ImGui::DragInt("targetRadialBlurNumSamples", &targetRadialBlur_.numSamples, 1);
	ImGui::DragFloat("targetRadialBlurWidth", &targetRadialBlur_.width, 0.1f);
	Easing::SelectEasingType(blurEasingType_);
}

void PlayerSwitchAllyState::ApplyJson(const Json& data) {

	deltaTimeScaleTime_ = JsonAdapter::GetValue<float>(data, "deltaTimeScaleTime_");
	switchAllyTime_ = JsonAdapter::GetValue<float>(data, "switchAllyTime_");
	deltaTimeScale_ = JsonAdapter::GetValue<float>(data, "deltaTimeScale_");
	deltaTimeScaleEasingType_ = static_cast<EasingType>(
		JsonAdapter::GetValue<int>(data, "deltaTimeScaleEasingType_"));
	blurTime_ = JsonAdapter::GetValue<float>(data, "blurTime_");
	blurEasingType_ = static_cast<EasingType>(
		JsonAdapter::GetValue<int>(data, "blurEasingType_"));
	targetRadialBlur_.center.y = JsonAdapter::GetValue<float>(data, "targetRadialBlur_.center.y");
	targetRadialBlur_.numSamples = JsonAdapter::GetValue<int>(data, "targetRadialBlur_.numSamples");
	targetRadialBlur_.width = JsonAdapter::GetValue<float>(data, "targetRadialBlur_.width");
}

void PlayerSwitchAllyState::SaveJson(Json& data) {

	data["deltaTimeScaleTime_"] = deltaTimeScaleTime_;
	data["switchAllyTime_"] = switchAllyTime_;
	data["deltaTimeScale_"] = deltaTimeScale_;
	data["deltaTimeScaleEasingType_"] = static_cast<int>(deltaTimeScaleEasingType_);
	data["blurTime_"] = blurTime_;
	data["blurEasingType_"] = static_cast<int>(blurEasingType_);
	data["targetRadialBlur_.center.y"] = targetRadialBlur_.center.y;
	data["targetRadialBlur_.numSamples"] = targetRadialBlur_.numSamples;
	data["targetRadialBlur_.width"] = targetRadialBlur_.width;
}