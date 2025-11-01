#include "TitleViewCamera.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Timer/GameTimer.h>

//============================================================================
//	TitleViewCamera classMethods
//============================================================================

void TitleViewCamera::Init() {

	initRotateX_ = transform_.eulerRotate.x;

	// json適応
	ApplyJson();
}

void TitleViewCamera::Update() {

	// Y軸回転を加算
	transform_.eulerRotate.y += rotateSpeed_ * GameTimer::GetDeltaTime();

	// オフセット距離
	Vector3 offset = Vector3::Transform(Vector3(0.0f, 0.0f, -viewOffset_),
		Matrix4x4::MakeRotateMatrix(transform_.eulerRotate));
	// 座標を設定
	transform_.translation = viewPoint_ + offset;

	// 行列更新
	BaseCamera::UpdateView();
}

void TitleViewCamera::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	BaseCamera::ImGui();

	ImGui::SeparatorText("Rotate");

	ImGui::DragFloat("rotateSpeed", &rotateSpeed_, 0.01f);
	if (ImGui::DragFloat("eulerRotateX", &eulerRotateX_, 0.01f)) {

		transform_.eulerRotate.x = eulerRotateX_;
	}
	ImGui::DragFloat3("viewPoint", &viewPoint_.x, 0.1f);
	ImGui::DragFloat3("viewOffset", &viewOffset_, 0.1f);

	LineRenderer::GetInstance()->DrawSphere(8, 4.0f,
		viewPoint_, Color::Cyan());
	LineRenderer::GetInstance()->DrawLine3D(viewPoint_,
		transform_.translation, Color::Cyan());
}

void TitleViewCamera::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Camera/Title/titleViewCameraParam.json", data)) {
		return;
	}

	rotateSpeed_ = data.value("rotateSpeed_", 0.1f);
	eulerRotateX_ = data.value("eulerRotateX", 1.0f);
	transform_.eulerRotate.x = eulerRotateX_;
	fovY_ = data.value("fovY_", 0.1f);
	farClip_ = data.value("farClip_", 0.1f);
	viewPoint_ = Vector3::FromJson(data["viewPoint_"]);
	viewOffset_ = data.value("viewOffset_", 32.0f);
}

void TitleViewCamera::SaveJson() {

	Json data;

	data["rotateSpeed_"] = rotateSpeed_;
	data["eulerRotateX"] = transform_.eulerRotate.x;
	data["fovY_"] = fovY_;
	data["farClip_"] = farClip_;
	data["viewPoint_"] = viewPoint_.ToJson();
	data["viewOffset_"] = viewOffset_;

	JsonAdapter::Save("Camera/Title/titleViewCameraParam.json", data);
}