#include "PlayerSwitchAllyState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Core/PostProcessSystem.h>
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

	canExit_ = false;

	// Noneで初期化
	selectState_ = PlayerState::None;
}

void PlayerSwitchAllyState::Enter(Player& player) {

	// Noneで初期化
	selectState_ = PlayerState::None;

	// カメラの状態を切り替え待ち状態にする
	followCamera_->SetState(FollowCameraState::SwitchAlly);
	// 画面シェイクを止める
	followCamera_->SetOverlayState(FollowCameraOverlayState::Shake, false);

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

	// 入力受付待ち
	switchAllyTimer_ += GameTimer::GetDeltaTime();
	float t = switchAllyTimer_ / switchAllyTime_;

	// 入力状態の確認
	CheckInput(t);

	// 遷移可能(何かを選択した)になったらHUDを元に戻す
	if (canExit_) {

		player.GetHUD()->SetValid();
		player.GetStunHUD()->SetCancel();
	}
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

	// リセット
	deltaTimeScaleTimer_ = 0.0f;
	switchAllyTimer_ = 0.0f;
	canExit_ = false;
}

void PlayerSwitchAllyState::ImGui([[maybe_unused]] const Player& player) {

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());
	ImGui::DragFloat("deltaTimeScaleTime", &deltaTimeScaleTime_, 0.01f);
	ImGui::DragFloat("switchAllyTime", &switchAllyTime_, 0.01f);
	ImGui::DragFloat("deltaTimeScale", &deltaTimeScale_, 0.01f);
}

void PlayerSwitchAllyState::ApplyJson(const Json& data) {

	deltaTimeScaleTime_ = JsonAdapter::GetValue<float>(data, "deltaTimeScaleTime_");
	switchAllyTime_ = JsonAdapter::GetValue<float>(data, "switchAllyTime_");
	deltaTimeScale_ = JsonAdapter::GetValue<float>(data, "deltaTimeScale_");
	deltaTimeScaleEasingType_ = static_cast<EasingType>(
		JsonAdapter::GetValue<int>(data, "deltaTimeScaleEasingType_"));
}

void PlayerSwitchAllyState::SaveJson(Json& data) {

	data["deltaTimeScaleTime_"] = deltaTimeScaleTime_;
	data["switchAllyTime_"] = switchAllyTime_;
	data["deltaTimeScale_"] = deltaTimeScale_;
	data["deltaTimeScaleEasingType_"] = static_cast<int>(deltaTimeScaleEasingType_);
}