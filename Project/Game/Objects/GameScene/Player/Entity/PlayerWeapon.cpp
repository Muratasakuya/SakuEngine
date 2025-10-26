#include "PlayerWeapon.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	PlayerWeapon classMethods
//============================================================================

void PlayerWeapon::SetInitTransform() {

	transform_->scale = initTransform_.scale;
	transform_->eulerRotate = initTransform_.eulerRotate;
	transform_->rotation = initTransform_.rotation;
	transform_->translation = initTransform_.translation;
}

void PlayerWeapon::Update() {

	// 剣先の座標を更新する
	tipTranslation_ = Vector3::Transform(tipOffset_, transform_->matrix.world);

	// 衝突情報更新
	Collider::UpdateAllBodies(*transform_);
}

void PlayerWeapon::DerivedImGui() {

	initTransform_.ImGui(itemWidth_);
	ImGui::Text("tipTranslation: %.3f,%.3f,%.3f", tipTranslation_.x,
		tipTranslation_.y, tipTranslation_.z);
	ImGui::DragFloat3("tipOffset", &tipOffset_.x, 0.01f);
	SetInitTransform();

	LineRenderer::GetInstance()->DrawSphere(6, 0.8f, tipTranslation_, Color::Cyan());

	Collider::ImGui(itemWidth_);
}

void PlayerWeapon::ApplyJson(const Json& data) {

	initTransform_.FromJson(data["InitTransform"]);
	tipOffset_ = Vector3::FromJson(data.value("tipOffset_", Json()));
	SetInitTransform();

	GameObject3D::ApplyMaterial(data);
}

void PlayerWeapon::SaveJson(Json& data) {

	GameObject3D::SaveMaterial(data);
	Collider::SaveBodyOffset(data);
	initTransform_.ToJson(data["InitTransform"]);
	data["tipOffset_"] = tipOffset_.ToJson();
}