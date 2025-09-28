#include "FollowCameraAllyAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	FollowCameraAllyAttackState classMethods
//============================================================================

FollowCameraAllyAttackState::FollowCameraAllyAttackState(float targetFovY) {

	targetFovY_ = targetFovY;
	canExit_ = false;
}

void FollowCameraAllyAttackState::Enter([[maybe_unused]] FollowCamera& followCamera) {
}

void FollowCameraAllyAttackState::Update(FollowCamera& followCamera) {

	// 味方に追従していく処理
	Vector3 rotation = followCamera.GetTransform().eulerRotate;
	Vector3 translation = followCamera.GetTransform().translation;
	Vector3 offset{};

	// 補間先の座標を補完割合に応じて補間する
	interTarget_ = Vector3::Lerp(interTarget_, targets_[FollowCameraTargetType::PlayerAlly]->GetWorldPos(), lerpRate_);

	Matrix4x4 rotateMatrix = Matrix4x4::MakeRotateMatrix(rotation);
	offset = Vector3::TransferNormal(offsetTranslation_, rotateMatrix);

	// offset分座標をずらす
	translation = interTarget_ + offset;
	followCamera.SetTranslation(translation);

	if (!startFovY_.has_value()) {

		startFovY_ = followCamera.GetFovY();
	}

	// 時間経過したら補間終了
	if (fovYLerpTime_ < fovYLerpTimer_) {

		followCamera.SetFovY(targetFovY_);
		return;
	}

	// 画角を補間する(元の画角に戻していく)
	fovYLerpTimer_ += GameTimer::GetDeltaTime();
	float lerpT = fovYLerpTimer_ / fovYLerpTime_;
	lerpT = EasedValue(fovYLerpEasingType_, lerpT);

	float fovY = std::lerp(startFovY_.value(), targetFovY_, lerpT);
	followCamera.SetFovY(fovY);
}

void FollowCameraAllyAttackState::Exit() {

	// リセット
	fovYLerpTimer_ = 0.0f;
	canExit_ = false;
}

void FollowCameraAllyAttackState::ImGui([[maybe_unused]] const FollowCamera& followCamera) {

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());

	ImGui::DragFloat3("offsetTranslation", &offsetTranslation_.x, 0.1f);
	ImGui::DragFloat("lerpRate", &lerpRate_, 0.01f);

	ImGui::DragFloat("fovYLerpTime", &fovYLerpTime_, 0.01f);
	Easing::SelectEasingType(fovYLerpEasingType_);
}

void FollowCameraAllyAttackState::ApplyJson(const Json& data) {

	offsetTranslation_ = JsonAdapter::ToObject<Vector3>(data["offsetTranslation_"]);
	lerpRate_ = JsonAdapter::GetValue<float>(data, "lerpRate_");
	fovYLerpTime_ = JsonAdapter::GetValue<float>(data, "fovYLerpTime_");
	fovYLerpEasingType_ = static_cast<EasingType>(
		JsonAdapter::GetValue<int>(data, "fovYLerpEasingType_"));
}

void FollowCameraAllyAttackState::SaveJson(Json& data) {

	data["fovYLerpTime_"] = fovYLerpTime_;
	data["lerpRate_"] = lerpRate_;
	data["fovYLerpEasingType_"] = static_cast<int>(fovYLerpEasingType_);
	data["offsetTranslation_"] = JsonAdapter::FromObject<Vector3>(offsetTranslation_);
}