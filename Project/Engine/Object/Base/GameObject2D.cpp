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

void GameObject2D::SetWindowSize() {

	// ウィンドウサイズに設定
	transform_->size = Vector2(Config::kWindowWidthf, Config::kWindowHeightf);
}

bool GameObject2D::ImGuiSize() {

	bool edit = false;

	edit |= ImGui::DragFloat2((tag_->name + "_size").c_str(), &transform_->size.x, 0.1f);
	edit |= ImGui::DragFloat2((tag_->name + "_textureSize").c_str(), &transform_->textureSize.x, 0.1f);

	return edit;
}