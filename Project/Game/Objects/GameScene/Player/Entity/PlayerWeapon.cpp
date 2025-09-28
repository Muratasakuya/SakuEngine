#include "PlayerWeapon.h"

//============================================================================
//	include
//============================================================================
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

	// 衝突情報更新
	Collider::UpdateAllBodies(*transform_);
}

void PlayerWeapon::DerivedImGui() {

	initTransform_.ImGui(itemWidth_);
	SetInitTransform();

	Collider::ImGui(itemWidth_);
}

void PlayerWeapon::ApplyJson(const Json& data) {

	initTransform_.FromJson(data["InitTransform"]);
	SetInitTransform();

	GameObject3D::ApplyMaterial(data);
}

void PlayerWeapon::SaveJson(Json& data) {

	GameObject3D::SaveMaterial(data);
	Collider::SaveBodyOffset(data);
	initTransform_.ToJson(data["InitTransform"]);
}