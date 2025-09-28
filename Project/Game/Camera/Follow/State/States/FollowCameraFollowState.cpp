#include "FollowCameraFollowState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	FollowCameraFollowState classMethods
//============================================================================

void FollowCameraFollowState::Enter([[maybe_unused]] FollowCamera& followCamera) {
}

void FollowCameraFollowState::Update(FollowCamera& followCamera) {

	InputType inputType = Input::GetInstance()->GetType();
	Vector3 rotation = followCamera.GetTransform().eulerRotate;
	Vector3 translation = followCamera.GetTransform().translation;
	Vector3 offset{};

	// 画角を常に元に戻しておく
	float fovY = std::lerp(followCamera.GetFovY(), defaultFovY_, fovYLerpRate_);
	followCamera.SetFovY(fovY);

	// 補間先の座標を補完割合に応じて補間する
	interTarget_ = Vector3::Lerp(interTarget_, targets_[FollowCameraTargetType::Player]->GetWorldPos(), lerpRate_);

	// 入力から移動量を取得する
	Vector2 rawInput(inputMapper_->GetVector(FollowCameraInputAction::RotateX),
		inputMapper_->GetVector(FollowCameraInputAction::RotateY));
	// 補間を適応する
	smoothedInput_ = Vector2::Lerp(smoothedInput_, rawInput, inputLerpRate_);

	// 現在の入力状態に応じて加算する
	if (inputType == InputType::Keyboard) {

		// Y軸回転: 左右
		rotation.y += smoothedInput_.x * mouseSensitivity_.y;
		// X軸回転: 上下
		rotation.x += smoothedInput_.y * mouseSensitivity_.x;
	} else if (inputType == InputType::GamePad) {

		// Y軸回転: 左右
		rotation.y += smoothedInput_.x * padSensitivity_.y;
		// X軸回転: 上下
		rotation.x -= smoothedInput_.y * padSensitivity_.x;
	}
	// 回転を制限する
	rotation.x = std::clamp(rotation.x, rotateMinusParam_.rotateClampX, rotatePlusParam_.rotateClampX);
	rotation.z = std::lerp(rotation.z, 0.0f, rotateZLerpRate_);
	followCamera.SetEulerRotation(rotation);

	// カメラ角度の距離を計算
	float distanceToMinus = std::abs(rotation.x - rotateMinusParam_.rotateClampX);
	float distanceToPlus = std::abs(rotation.x - rotatePlusParam_.rotateClampX);
	// 目標値
	float targetZ = defaultOffsetZ_;
	// clampThresholdより近ければ近いほど補間される
	if (distanceToMinus < rotateMinusParam_.clampThreshold) {

		float t = 1.0f - (distanceToMinus / rotateMinusParam_.clampThreshold);
		targetZ = std::lerp(defaultOffsetZ_, rotateMinusParam_.offsetZNear, t);
	} else if (distanceToPlus < rotatePlusParam_.clampThreshold) {

		float t = 1.0f - (distanceToPlus / rotatePlusParam_.clampThreshold);
		targetZ = std::lerp(defaultOffsetZ_, rotatePlusParam_.offsetZNear, t);
	}

	// オフセット距離補間
	offsetTranslation_.z = std::lerp(offsetTranslation_.z, targetZ, offsetZLerpRate_);
	offsetTranslation_.y = std::lerp(offsetTranslation_.y, defaultOffsetY_, offsetYLerpRate_);
	offsetTranslation_.x = std::lerp(offsetTranslation_.x, defaultOffsetX_, offsetXLerpRate_);

	Matrix4x4 rotateMatrix = Matrix4x4::MakeRotateMatrix(rotation);
	offset = Vector3::TransferNormal(offsetTranslation_, rotateMatrix);

	// offset分座標をずらす
	translation = interTarget_ + offset;
	followCamera.SetTranslation(translation);
}

void FollowCameraFollowState::Exit() {
}

void FollowCameraFollowState::ImGui([[maybe_unused]] const FollowCamera& followCamera) {

	ImGui::DragFloat3("offsetTranslation", &offsetTranslation_.x, 0.1f);

	ImGui::DragFloat("lerpRate", &lerpRate_, 0.1f);
	ImGui::DragFloat("inputLerpRate_", &inputLerpRate_, 0.01f);

	ImGui::DragFloat2("mouseSensitivity", &mouseSensitivity_.x, 0.001f);
	ImGui::DragFloat2("padSensitivity", &padSensitivity_.x, 0.001f);
	ImGui::DragFloat2("smoothedInput", &smoothedInput_.x, 0.001f);

	ImGui::DragFloat("fovYLerpRate", &fovYLerpRate_, 0.001f);
	ImGui::DragFloat("offsetZLerpRate", &offsetZLerpRate_, 0.001f);
	ImGui::DragFloat("offsetYLerpRate", &offsetYLerpRate_, 0.001f);
	ImGui::DragFloat("offsetXLerpRate", &offsetXLerpRate_, 0.001f);
	ImGui::DragFloat("rotateZLerpRate", &rotateZLerpRate_, 0.001f);
	ImGui::DragFloat("rotatePlusParam.rotateClampX", &rotatePlusParam_.rotateClampX, 0.001f);
	ImGui::DragFloat("rotatePlusParam.offsetZNear", &rotatePlusParam_.offsetZNear, 0.001f);
	ImGui::DragFloat("rotatePlusParam.clampThreshold", &rotatePlusParam_.clampThreshold, 0.001f);

	ImGui::DragFloat("rotateMinusParam.rotateClampX", &rotateMinusParam_.rotateClampX, 0.001f);
	ImGui::DragFloat("rotateMinusParam.offsetZNear", &rotateMinusParam_.offsetZNear, 0.001f);
	ImGui::DragFloat("rotateMinusParam.clampThreshold", &rotateMinusParam_.clampThreshold, 0.001f);
}

void FollowCameraFollowState::ApplyJson(const Json& data) {

	offsetTranslation_ = JsonAdapter::ToObject<Vector3>(data["offsetTranslation_"]);
	defaultOffsetZ_ = offsetTranslation_.z;
	defaultOffsetY_ = offsetTranslation_.y;

	lerpRate_ = JsonAdapter::GetValue<float>(data, "lerpRate_");
	inputLerpRate_ = JsonAdapter::GetValue<float>(data, "inputLerpRate_");
	mouseSensitivity_ = JsonAdapter::ToObject<Vector2>(data["mouseSensitivity_"]);
	padSensitivity_ = JsonAdapter::ToObject<Vector2>(data["padSensitivity_"]);

	fovYLerpRate_ = JsonAdapter::GetValue<float>(data, "fovYLerpRate_");
	offsetZLerpRate_ = JsonAdapter::GetValue<float>(data, "offsetZLerpRate_");
	offsetYLerpRate_ = JsonAdapter::GetValue<float>(data, "offsetYLerpRate_");
	rotateZLerpRate_ = JsonAdapter::GetValue<float>(data, "rotateZLerpRate_");
	offsetXLerpRate_ = JsonAdapter::GetValue<float>(data, "offsetXLerpRate_");

	rotatePlusParam_.rotateClampX = JsonAdapter::GetValue<float>(data, "rotateClampPlusX_");
	rotatePlusParam_.offsetZNear = JsonAdapter::GetValue<float>(data, "rotatePlusParam_.offsetZNear");
	rotatePlusParam_.clampThreshold = JsonAdapter::GetValue<float>(data, "rotatePlusParam_.clampThreshold");

	rotateMinusParam_.rotateClampX = JsonAdapter::GetValue<float>(data, "rotateClampMinusX_");
	rotateMinusParam_.offsetZNear = JsonAdapter::GetValue<float>(data, "rotateMinusParam_.offsetZNear");
	rotateMinusParam_.clampThreshold = JsonAdapter::GetValue<float>(data, "rotateMinusParam_.clampThreshold");
}

void FollowCameraFollowState::SaveJson(Json& data) {

	data["offsetTranslation_"] = JsonAdapter::FromObject<Vector3>(offsetTranslation_);
	data["lerpRate_"] = lerpRate_;
	data["inputLerpRate_"] = inputLerpRate_;
	data["mouseSensitivity_"] = JsonAdapter::FromObject<Vector2>(mouseSensitivity_);
	data["padSensitivity_"] = JsonAdapter::FromObject<Vector2>(padSensitivity_);

	data["fovYLerpRate_"] = fovYLerpRate_;
	data["offsetZLerpRate_"] = offsetZLerpRate_;
	data["offsetYLerpRate_"] = offsetYLerpRate_;
	data["rotateZLerpRate_"] = rotateZLerpRate_;
	data["offsetXLerpRate_"] = offsetXLerpRate_;

	data["rotateClampPlusX_"] = rotatePlusParam_.rotateClampX;
	data["rotatePlusParam_.offsetZNear"] = rotatePlusParam_.offsetZNear;
	data["rotatePlusParam_.clampThreshold"] = rotatePlusParam_.clampThreshold;

	data["rotateClampMinusX_"] = rotateMinusParam_.rotateClampX;
	data["rotateMinusParam_.offsetZNear"] = rotateMinusParam_.offsetZNear;
	data["rotateMinusParam_.clampThreshold"] = rotateMinusParam_.clampThreshold;
}

void FollowCameraFollowState::SetOffsetTranslation(const Vector3& translation) {

	offsetTranslation_ = translation;
}