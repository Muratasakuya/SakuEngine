#include "ClearResultCamera.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Timer/GameTimer.h>

//============================================================================
//	ClearResultCamera classMethods
//============================================================================

void ClearResultCamera::Init() {

	initRotateX_ = transform_.eulerRotate.x;

	// json適応
	ApplyJson();

	// 初期化値設定
	currentState_ = State::Begin;
}

void ClearResultCamera::Update() {

	switch (currentState_) {
	case ClearResultCamera::State::Begin:

		UpdateAnimation();
		break;
	case ClearResultCamera::State::Rotate:

		UpdateRotate();
		break;
	}

	// 行列更新
	BaseCamera::UpdateView();
}

void ClearResultCamera::UpdateAnimation() {

	// 時間を進める
	animationTimer_.Update();

	// 座標を補間
	transform_.translation = Vector3::Lerp(
		startPos_, targetPos_, animationTimer_.easedT_);

	// 補間が終了したら次に進める
	if (animationTimer_.IsReached()) {

		animationTimer_.Reset();
		currentState_ = State::Rotate;
		transform_.eulerRotate.x = eulerRotateX_;
	}
}

void ClearResultCamera::UpdateRotate() {

	// Y軸回転を加算
	transform_.eulerRotate.y += rotateSpeed_ * GameTimer::GetDeltaTime();

	// オフセット距離
	Vector3 offset = Vector3::Transform(Vector3(0.0f, 0.0f, -viewOffset_),
		Matrix4x4::MakeRotateMatrix(transform_.eulerRotate));
	// 座標を設定
	transform_.translation = viewPoint_ + offset;
}

void ClearResultCamera::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	EnumAdapter<State>::Combo("state", &currentState_);

	BaseCamera::ImGui();

	ImGui::SeparatorText("Begin");

	if (ImGui::Button("Reste")) {

		animationTimer_.Reset();
	}

	ImGui::DragFloat3("startPos", &startPos_.x, 0.1f);
	ImGui::DragFloat3("targetPos", &targetPos_.x, 0.1f);

	animationTimer_.ImGui("Animation");

	ImGui::SeparatorText("Rotate");

	ImGui::DragFloat("rotateSpeed", &rotateSpeed_, 0.01f);
	if (ImGui::DragFloat("eulerRotateX", &eulerRotateX_, 0.01f)) {

		transform_.eulerRotate.x = eulerRotateX_;
	}
	ImGui::DragFloat3("viewPoint", &viewPoint_.x, 0.1f);
	ImGui::DragFloat3("viewOffset", &viewOffset_, 0.1f);

	switch (currentState_) {
	case ClearResultCamera::State::Begin: {

		transform_.eulerRotate.x = initRotateX_;
		transform_.eulerRotate.y = 0.0f;
		break;
	}
	case ClearResultCamera::State::Rotate: {

		LineRenderer::GetInstance()->DrawSphere(8, 4.0f,
			viewPoint_, Color::Cyan());
		LineRenderer::GetInstance()->DrawLine3D(viewPoint_,
			transform_.translation, Color::Cyan());
		break;
	}
	}
}

void ClearResultCamera::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Camera/Clear/resultCameraParam.json", data)) {
		return;
	}

	animationTimer_.FromJson(data["AnimationTimer"]);
	startPos_ = Vector3::FromJson(data["startPos_"]);
	targetPos_ = Vector3::FromJson(data["targetPos_"]);

	rotateSpeed_ = data.value("rotateSpeed_", 0.1f);
	eulerRotateX_ = data.value("eulerRotateX", 1.0f);
	fovY_ = data.value("fovY_", 0.1f);
	farClip_ = data.value("farClip_", 0.1f);
	viewPoint_ = Vector3::FromJson(data["viewPoint_"]);
	viewOffset_ = data.value("viewOffset_", 32.0f);
}

void ClearResultCamera::SaveJson() {

	Json data;

	animationTimer_.ToJson(data["AnimationTimer"]);
	data["startPos_"] = startPos_.ToJson();
	data["targetPos_"] = targetPos_.ToJson();

	data["rotateSpeed_"] = rotateSpeed_;
	data["eulerRotateX"] = transform_.eulerRotate.x;
	data["fovY_"] = fovY_;
	data["farClip_"] = farClip_;
	data["viewPoint_"] = viewPoint_.ToJson();
	data["viewOffset_"] = viewOffset_;

	JsonAdapter::Save("Camera/Clear/resultCameraParam.json", data);
}