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

	// 画角に変更があっても常に初期値に補間させる
	float fovY = std::lerp(followCamera.GetFovY(), defaultFovY_, fovYLerpRate_);
	followCamera.SetFovY(fovY);

	// 追従ターゲット位置の補間
	interTarget_ = Vector3::Lerp(interTarget_,
		targets_[FollowCameraTargetType::Player]->GetWorldPos(), lerpRate_);

	// 入力から移動量を取得する
	InputType inputType = Input::GetInstance()->GetType();
	Vector2 rawInput(inputMapper_->GetVector(FollowCameraInputAction::RotateX),
		inputMapper_->GetVector(FollowCameraInputAction::RotateY));
	// 補間を適応する
	smoothedInput_ = Vector2::Lerp(smoothedInput_, rawInput, inputLerpRate_);

	// X軸とY軸の入力量
	// Y軸
	float yawDelta = (inputType == InputType::Keyboard) ?
		(smoothedInput_.x * mouseSensitivity_.y) :
		(smoothedInput_.x * padSensitivity_.y);
	// X軸
	float pitchDelta = (inputType == InputType::Keyboard) ?
		(smoothedInput_.y * mouseSensitivity_.x) :
		(-smoothedInput_.y * padSensitivity_.x);

	Quaternion currentRotation = followCamera.GetTransform().rotation;
	// Y軸の回転
	Quaternion yawRotation = Quaternion::Normalize(Quaternion::MakeRotateAxisAngleQuaternion(
		Direction::Get(Direction3D::Up), yawDelta) * currentRotation);
	// X軸の回転
	Vector3 rightAxis = (yawRotation * Direction::Get(Direction3D::Right)).Normalize();
	Quaternion pitchRotation = Quaternion::Normalize(
		Quaternion::MakeRotateAxisAngleQuaternion(rightAxis, pitchDelta));
	// X軸回転とY軸回転を合成
	Quaternion candidateRotation = Quaternion::Normalize(pitchRotation * yawRotation);

	// 前方ベクトルからX軸回転を取得
	Vector3 forward = candidateRotation * Direction::Get(Direction3D::Forward);
	float currentPitch = std::atan2(forward.y, std::sqrt(forward.x * forward.x + forward.z * forward.z));
	// X軸の回転を制御
	float pitchClamped = std::clamp(currentPitch,
		rotateMinusParam_.rotateClampX, rotatePlusParam_.rotateClampX);
	// 回転をクランプしたら回転を再設定する
	if (pitchClamped != currentPitch) {

		Vector3 horizontal = Vector3(forward.x, 0.0f, forward.z);
		if (horizontal.Length() < 1e-6f) {

			Vector3 fowrardYaw = yawRotation * Direction::Get(Direction3D::Forward);
			horizontal = Vector3(fowrardYaw.x, 0.0f, fowrardYaw.z);
		}
		horizontal = horizontal.Normalize();

		Vector3 forwardClamped = Vector3(horizontal.x * std::cos(pitchClamped),
			std::sin(pitchClamped), horizontal.z * std::cos(pitchClamped)).Normalize();
		candidateRotation = Quaternion::LookRotation(forwardClamped, Direction::Get(Direction3D::Up));
	}

	// Z回転を常に0.0fに補間
	Vector3 forwardRoll = candidateRotation * Direction::Get(Direction3D::Forward);
	Quaternion currentRoll = Quaternion::LookRotation(
		forwardRoll.Normalize(), Direction::Get(Direction3D::Up));
	Quaternion rotation = Quaternion::Slerp(candidateRotation, currentRoll, rotateZLerpRate_);

	// クランプされるまでの距離
	float distanceToMinus = std::fabs(pitchClamped - rotateMinusParam_.rotateClampX);
	float distanceToPlus = std::fabs(pitchClamped - rotatePlusParam_.rotateClampX);
	float targetZ = defaultOffsetZ_;
	// 距離が閾値以下なら補間値分最大/最小の回転に近づける
	if (distanceToMinus < rotateMinusParam_.clampThreshold) {

		float t = 1.0f - (distanceToMinus / rotateMinusParam_.clampThreshold);
		targetZ = std::lerp(defaultOffsetZ_, rotateMinusParam_.offsetZNear, t);
	} else if (distanceToPlus < rotatePlusParam_.clampThreshold) {

		float t = 1.0f - (distanceToPlus / rotatePlusParam_.clampThreshold);
		targetZ = std::lerp(defaultOffsetZ_, rotatePlusParam_.offsetZNear, t);
	}

	// オフセットの補間
	offsetTranslation_.z = std::lerp(offsetTranslation_.z, targetZ, offsetZLerpRate_);
	offsetTranslation_.y = std::lerp(offsetTranslation_.y, defaultOffsetY_, offsetYLerpRate_);
	offsetTranslation_.x = std::lerp(offsetTranslation_.x, defaultOffsetX_, offsetXLerpRate_);

	// 回転を考慮したオフセットと追従先の座標を足す
	Vector3 translation = interTarget_ + rotation * offsetTranslation_;

	// カメラにセット
	followCamera.SetRotation(rotation);
	followCamera.SetTranslation(translation);
}

void FollowCameraFollowState::Exit() {
}

void FollowCameraFollowState::ImGui([[maybe_unused]] const FollowCamera& followCamera) {

	ImGui::DragFloat3("offsetTranslation", &offsetTranslation_.x, 0.1f);

	ImGui::DragFloat("lerpRate", &lerpRate_, 0.01f);
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