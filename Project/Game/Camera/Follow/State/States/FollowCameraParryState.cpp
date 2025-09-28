#include "FollowCameraParryState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	FollowCameraParryState classMethods
//============================================================================

void FollowCameraParryState::Enter(FollowCamera& followCamera) {

	// 補間開始値を設定
	const Vector3& cameraRotation = followCamera.GetTransform().eulerRotate;
	startRotate_ = cameraRotation;
	// Y軸回転は元の回転に合わせる
	targetRotate_.y = cameraRotation.y;

	lerpTimer_ = 0.0f;
	waitTimer_ = 0.0f;
	canExit_ = false;
}

void FollowCameraParryState::Update(FollowCamera& followCamera) {

	// 時間経過を進める
	lerpTimer_ += GameTimer::GetDeltaTime();
	float lerpT = lerpTimer_ / lerpTime_;
	float easedT = EasedValue(easingType_, lerpT);

	// 補間が終了したら待機状態に遷移させる
	if (1.0f <= lerpT) {

		// 時間経過を進める
		waitTimer_ += GameTimer::GetDeltaTime();

		// 時間経過が終了したら状態を終了させる
		if (waitTime_ < waitTimer_) {

			canExit_ = true;
		}
		easedT = 1.0f;
	}

	// 距離
	Vector3 offsetTranslation = Vector3::Lerp(
		startOffsetTranslation_, targetOffsetTranslation_, easedT);
	// 回転
	Vector3 rotation = Vector3::Lerp(
		startRotate_, targetRotate_, easedT);
	// 画角
	float fovY = std::lerp(startFovY_, targetFovY_, easedT);

	// 補間値を設定
	// 画角
	followCamera.SetFovY(fovY);

	// 回転
	followCamera.SetEulerRotation(rotation);

	// 座標
	// 補間先の座標を補間割合に応じて補間する
	interTarget_ = Vector3::Lerp(interTarget_, targets_[FollowCameraTargetType::Player]->GetWorldPos(), lerpRate_);

	Matrix4x4 rotateMatrix = Matrix4x4::MakeRotateMatrix(rotation);
	Vector3 offset = Vector3::TransferNormal(offsetTranslation, rotateMatrix);

	// offset分座標をずらす
	Vector3 translation = interTarget_ + offset;
	followCamera.SetTranslation(translation);
}

void FollowCameraParryState::Exit() {

	// リセット
	lerpTimer_ = 0.0f;
	waitTimer_ = 0.0f;
	interTarget_.Init();
	canExit_ = false;
}

void FollowCameraParryState::ImGui([[maybe_unused]] const FollowCamera& followCamera) {

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());
	ImGui::Text(std::format("lerpTimer: {}", lerpTimer_).c_str());
	ImGui::Text(std::format("waitTimer: {}", waitTimer_).c_str());

	ImGui::DragFloat3("targetOffsetTranslation", &targetOffsetTranslation_.x, 0.1f);
	ImGui::DragFloat3("targetRotate", &targetRotate_.x, 0.01f);

	ImGui::DragFloat("lerpTime", &lerpTime_, 0.01f);
	ImGui::DragFloat("waitTime", &waitTime_, 0.01f);
	ImGui::DragFloat("lerpRate##FollowCameraParryState", &lerpRate_, 0.1f);
	ImGui::DragFloat("targetFovY", &targetFovY_, 0.01f);

	Easing::SelectEasingType(easingType_);
}

void FollowCameraParryState::ApplyJson(const Json& data) {

	targetOffsetTranslation_ = JsonAdapter::ToObject<Vector3>(data["targetOffsetTranslation_"]);
	targetRotate_ = JsonAdapter::ToObject<Vector3>(data["targetRotate_"]);
	lerpTime_ = JsonAdapter::GetValue<float>(data, "lerpTime_");
	waitTime_ = JsonAdapter::GetValue<float>(data, "waitTime_");
	lerpRate_ = JsonAdapter::GetValue<float>(data, "lerpRate_");
	targetFovY_ = JsonAdapter::GetValue<float>(data, "targetFovY_");
	easingType_ = JsonAdapter::GetValue<EasingType>(data, "easingType_");
}

void FollowCameraParryState::SaveJson(Json& data) {

	data["targetOffsetTranslation_"] = JsonAdapter::FromObject<Vector3>(targetOffsetTranslation_);
	data["targetRotate_"] = JsonAdapter::FromObject<Vector3>(targetRotate_);
	data["lerpTime_"] = lerpTime_;
	data["waitTime_"] = waitTime_;
	data["lerpRate_"] = lerpRate_;
	data["targetFovY_"] = targetFovY_;
	data["easingType_"] = static_cast<int>(easingType_);
}

void FollowCameraParryState::SetStartOffsetTranslation(const Vector3& startOffsetTranslation) {

	startOffsetTranslation_ = startOffsetTranslation;
}