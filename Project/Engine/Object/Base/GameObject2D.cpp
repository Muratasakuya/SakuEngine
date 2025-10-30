#include "GameObject2D.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Editor/GameObject/ImGuiObjectEditor.h>
#include <Engine/Scene/Camera/BaseCamera.h>

//============================================================================
//	GameObject2D classMethods
//============================================================================

void GameObject2D::Init(const std::string& textureName,
	const std::string& name, const std::string& groupName) {

	// object作成
	objectId_ = objectManager_->CreateObject2D(textureName, name, groupName);

	// data取得
	transform_ = objectManager_->GetData<Transform2D>(objectId_);
	material_ = objectManager_->GetData<SpriteMaterial>(objectId_);
	sprite_ = objectManager_->GetData<Sprite>(objectId_);
	tag_ = objectManager_->GetData<ObjectTag>(objectId_);

	// editorに登録
	ImGuiObjectEditor::GetInstance()->Registerobject(objectId_, this);

	// 継承先のinit実装
	DerivedInit();
}

void GameObject2D::ImGui() {

	if (ImGui::BeginTabBar("Obj2DTab")) {
		if (ImGui::BeginTabItem("Sprite")) {

			sprite_->ImGui(itemWidth_);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Transform")) {

			transform_->ImGui(itemWidth_);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Material")) {

			material_->ImGui(itemWidth_);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Derived")) {

			// 継承先のimgui実装
			DerivedImGui();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void GameObject2D::ApplyJson(const Json& data) {

	ApplyTransform(data);
	ApplyMaterial(data);
	ApplySprite(data);
}

void GameObject2D::SaveJson(Json& data) {

	SaveTransform(data);
	SaveMaterial(data);
	SaveSprite(data);
}

void GameObject2D::ApplyTransform(const Json& data) {

	transform_->FromJson(data["Transform"]);
}

void GameObject2D::SaveTransform(Json& data) {

	transform_->ToJson(data["Transform"]);
}

void GameObject2D::ApplyMaterial(const Json& data) {

	material_->FromJson(data["Material"]);
}

void GameObject2D::SaveMaterial(Json& data) {

	material_->ToJson(data["Material"]);
}

void GameObject2D::ApplySprite(const Json& data) {

	sprite_->FromJson(data["Sprite"]);
}

void GameObject2D::SaveSprite(Json& data) {

	sprite_->ToJson(data["Sprite"]);
}

void GameObject2D::SetCenterTranslation() {

	// 画面中心に設定
	transform_->translation = Vector2(Config::kWindowWidthf / 2.0f, Config::kWindowHeightf / 2.0f);
}

void GameObject2D::ProjectToScreen(const Vector3& translation, const BaseCamera& camera) {

	Matrix4x4 viewMatrix = camera.GetViewMatrix();
	Matrix4x4 projectionMatrix = camera.GetProjectionMatrix();

	Vector3 viewPos = Vector3::Transform(translation, viewMatrix);
	Vector3 clipPos = Vector3::Transform(viewPos, projectionMatrix);

	float screenX = (clipPos.x * 0.5f + 0.5f) * Config::kWindowWidthf;
	float screenY = (1.0f - (clipPos.y * 0.5f + 0.5f)) * Config::kWindowHeightf;

	transform_->translation = Vector2(screenX, screenY);
}

bool GameObject2D::ImGuiSize() {

	bool edit = false;

	edit |= ImGui::DragFloat2((tag_->name + "_size").c_str(), &transform_->size.x, 0.1f);
	edit |= ImGui::DragFloat2((tag_->name + "_textureSize").c_str(), &transform_->textureSize.x, 0.1f);

	return edit;
}