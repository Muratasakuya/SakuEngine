#include "PlayerDashState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Camera/Follow/FollowCamera.h>

//============================================================================
//	PlayerDashState classMethods
//============================================================================

void PlayerDashState::Enter(Player& player) {

	player.SetNextAnimation("player_dash", true, nextAnimDuration_);

	// 加速開始
	currentState_ = State::Accel;
	accelLerp_->Start();

	// カメラを見やすい位置まで補間させる
	followCamera_->SetOverlayState(FollowCameraOverlayState::ReturnDefaultRotate, true);
}

void PlayerDashState::Update(Player& player) {

	// ダッシュ更新
	UpdateDash(player);
	// 回転、進行方向に向かせる
	SetRotateToDirection(player, move_);
}

void PlayerDashState::UpdateState() {

	switch (currentState_) {
	case PlayerDashState::State::Accel: {

		// 加速させる
		accelLerp_->LerpValue(moveSpeed_);
		// 加速が終了したら次の状態に遷移
		if (accelLerp_->IsFinished()) {

			currentState_ = State::Sustain;
		}
		break;
	}
	case PlayerDashState::State::Sustain: {

		sustainTimer_ += GameTimer::GetScaledDeltaTime();

		// 時間経過後減速させる
		if (sustainTime_ <= sustainTimer_) {

			currentState_ = State::Decel;
			decelLerp_->Start();
		}
		break;
	}
	case PlayerDashState::State::Decel: {

		decelLerp_->LerpValue(moveSpeed_);
		break;
	}
	}
}

void PlayerDashState::UpdateDash(Player& player) {

	// 速度の状態更新
	UpdateState();

	// 入力値取得
	Vector2 inputValue(inputMapper_->GetVector(PlayerInputAction::MoveX),
		inputMapper_->GetVector(PlayerInputAction::MoveZ));
	inputValue = Vector2::Normalize(inputValue);
	if (inputValue.Length() > epsilon_) {

		Vector3 direction = Vector3::Normalize(Vector3(inputValue.x, 0.0f, inputValue.y));
		direction = Vector3::TransferNormal(direction,
			Quaternion::MakeRotateMatrix(followCamera_->GetTransform().rotation));
		move_ = direction * moveSpeed_;
	}
	// 特に何も入力していなくても加速状態の時は向いている方向に加速分動かして進ませる
	else {

		move_ = player.GetTransform().GetForward() * moveSpeed_;
	}
	move_.y = 0.0f;

	// 座標を設定
	Vector3 translation = player.GetTranslation();
	translation += move_;
	player.SetTranslation(translation);
}

void PlayerDashState::Exit(Player& player) {

	// animationをリセット
	accelLerp_->Reset();
	decelLerp_->Reset();
	sustainTimer_ = 0.0f;
	move_.Init();

	player.ResetAnimation();
}

bool PlayerDashState::GetCanExit() const {

	if (inputMapper_->IsTriggered(PlayerInputAction::Attack)) {

		return true;
	}
	return !inputMapper_->IsPressed(PlayerInputAction::Dash);
}

void PlayerDashState::ImGui([[maybe_unused]] const Player& player) {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate_", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("sustainTime", &sustainTime_, 0.01f);
	if (ImGui::BeginTabBar("DashSpeedTabs")) {
		if (ImGui::BeginTabItem("Accel")) {

			accelLerp_->ImGui("accel");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Decel")) {

			decelLerp_->ImGui("decel");
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void PlayerDashState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	sustainTime_ = JsonAdapter::GetValue<float>(data, "sustainTime_");

	accelLerp_ = std::make_unique<SimpleAnimation<float>>();
	decelLerp_ = std::make_unique<SimpleAnimation<float>>();
	if (data.contains("accelLerp_")) {

		accelLerp_->FromJson(data["accelLerp_"]);
	}
	if (data.contains("decelLerp_")) {

		decelLerp_->FromJson(data["decelLerp_"]);
	}
}

void PlayerDashState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	accelLerp_->ToJson(data["accelLerp_"]);
	decelLerp_->ToJson(data["decelLerp_"]);
}