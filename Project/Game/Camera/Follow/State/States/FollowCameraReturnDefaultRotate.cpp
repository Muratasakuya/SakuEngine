#include "FollowCameraReturnDefaultRotate.h"

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/MathUtils.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

//============================================================================
//	FollowCameraReturnDefaultRotate classMethods
//============================================================================

FollowCameraReturnDefaultRotate::FollowCameraReturnDefaultRotate() {

	// 時間設定したい状態を追加
	targetTime_.emplace(PlayerState::Idle, 0.0f);
	targetTime_.emplace(PlayerState::Walk, 0.0f);
	targetTime_.emplace(PlayerState::Dash, 0.0f);
}

void FollowCameraReturnDefaultRotate::Enter(FollowCamera& followCamera) {

	// 現在のx軸回転が閾値以上なら処理しない
	// ローカルX軸を取り出して角度を計算
	startTwistX_ = Quaternion::ExtractTwistX(Quaternion::Normalize(followCamera.GetTransform().rotation));
	const float currentX = Math::AngleFromTwist(startTwistX_, Math::Axis::X);

	// しきい値以上なら処理しない
	if (lerpThreshold_ <= currentX) {
		canExit_ = true;
		return;
	}

	// リセット
	requestTargetTime_ = std::nullopt;
	lerpTimer_.Reset();

	// プレイヤーのの状態に応じて目標時間を設定、なければデフォルト
	if (Algorithm::Find(targetTime_, player_->GetCurrentState())) {

		requestTargetTime_ = targetTime_[player_->GetCurrentState()];
	}
}

void FollowCameraReturnDefaultRotate::Update(FollowCamera& followCamera) {

	// 時間を進める
	lerpTimer_.Update(requestTargetTime_);

	// 補間処理
	// y、z軸は維持した状態でx軸を補間する
	Quaternion currentRotation = Quaternion::Normalize(followCamera.GetTransform().rotation);
	Quaternion twistX = Quaternion::ExtractTwistX(currentRotation);
	Quaternion inverseTwistX = Quaternion::Inverse(twistX);
	Quaternion swing = Quaternion::Normalize(currentRotation * inverseTwistX);

	// 目標のx回転
	const Quaternion targetRotationX = Quaternion::Normalize(
		Quaternion::MakeAxisAngle(Direction::Get(Direction3D::Right), targetRotateX_));
	const Quaternion lerpRotationX =
		Quaternion::Slerp(startTwistX_, targetRotationX, lerpTimer_.easedT_);

	// 元のyz軸の回転と補間後のx軸回転を合成する
	const Quaternion rotation = Quaternion::Normalize(swing * lerpRotationX);

	// 値を設定
	followCamera.SetRotation(rotation);

	// 時間経過で処理終了
	if (lerpTimer_.IsReached()) {

		canExit_ = true;
	}
}

void FollowCameraReturnDefaultRotate::Exit() {

	// リセット
	canExit_ = false;
	lerpTimer_.Reset();
}

void FollowCameraReturnDefaultRotate::ImGui([[maybe_unused]] const FollowCamera& followCamera) {

	ImGui::DragFloat("targetRotateX", &targetRotateX_, 0.01f);
	ImGui::DragFloat("lerpThreshold", &lerpThreshold_, 0.01f);

	ImGui::SeparatorText("TargetTime");

	ImGui::Text(std::format("hasTargetTime: {}", requestTargetTime_.has_value()).c_str());
	if (requestTargetTime_.has_value()) {

		ImGui::Text("targetTime: %.3f", requestTargetTime_.value());
	}

	for (auto& [state, value] : targetTime_) {

		const char* name = EnumAdapter<PlayerState>::ToString(state);

		ImGui::PushID(name);

		ImGui::Text(name);
		ImGui::SameLine();
		ImGui::DragFloat("Time", &value, 0.01f);

		ImGui::PopID();
	}

	lerpTimer_.ImGui("Timer");
}

void FollowCameraReturnDefaultRotate::ApplyJson(const Json& data) {

	targetRotateX_ = data.value("targetRotateX_", 0.2f);
	lerpThreshold_ = data.value("lerpThreshold_", 0.0f);

	lerpTimer_.FromJson(data.value("Timer", Json()));

	if (data.contains("TargetTime")) {
		for (auto& [state, value] : targetTime_) {

			const char* key = EnumAdapter<PlayerState>::ToString(state);
			if (data["TargetTime"].contains(key)) {

				value = data["TargetTime"][key];
			}
		}
	}
}

void FollowCameraReturnDefaultRotate::SaveJson(Json& data) {

	data["targetRotateX_"] = targetRotateX_;
	data["lerpThreshold_"] = lerpThreshold_;

	lerpTimer_.ToJson(data["Timer"]);

	for (const auto& [state, value] : targetTime_) {

		const char* key = EnumAdapter<PlayerState>::ToString(state);
		data["TargetTime"][key] = value;
	}
}