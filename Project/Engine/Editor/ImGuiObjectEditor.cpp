#include "ImGuiObjectEditor.h"

//============================================================================
//	imgui
//============================================================================
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/System/Systems/TagSystem.h>
#include <Engine/Object/Base/Interface/IGameObject.h>
#include <Lib/MathUtils/Algorithm.h>
#include <Lib/MathUtils/MathUtils.h>

// data
#include <Engine/Object/Data/Transform.h>
#include <Engine/Object/Data/Material.h>
#include <Engine/Object/Data/Animation.h>
#include <Engine/Object/Data/ObjectTag.h>
#include <Engine/Object/Data/Sprite.h>
#include <Engine/Object/Data/Skybox.h>

//============================================================================
//	ImGuiObjectEditor classMethods
//============================================================================

ImGuiObjectEditor* ImGuiObjectEditor::instance_ = nullptr;

ImGuiObjectEditor* ImGuiObjectEditor::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new ImGuiObjectEditor();
	}
	return instance_;
}

void ImGuiObjectEditor::Finalize() {

	if (instance_ != nullptr) {

		delete instance_;
		instance_ = nullptr;
	}
}

void ImGuiObjectEditor::Init() {

	objectManager_ = nullptr;
	objectManager_ = ObjectManager::GetInstance();
	tagSystem_ = nullptr;
	tagSystem_ = objectManager_->GetSystem<TagSystem>();
}

void ImGuiObjectEditor::SelectById(uint32_t id) {

	// フォーカス先を設定
	if (Is3D(id)) {

		selected3D_ = id;
		selected2D_.reset();
	} else if (Is2D(id)) {

		selected2D_ = id;
		selected3D_.reset();
	}
}

void ImGuiObjectEditor::SelectObject() {

	CreateGroup();
	SelectGroupedObject();
}

bool ImGuiObjectEditor::Is3D(uint32_t object) const {

	return objectManager_->GetData<Transform3D>(object) != nullptr ||
		objectManager_->GetData<Skybox>(object) != nullptr;
}

bool ImGuiObjectEditor::Is2D(uint32_t object) const {

	return objectManager_->GetData<Transform2D>(object) != nullptr;
}

void ImGuiObjectEditor::DrawSelectable(uint32_t object, const std::string& name) {

	if (Is3D(object)) {
		bool selected = (selected3D_ == object);
		std::string label = name + "##3D" + std::to_string(object);
		if (ImGui::Selectable(label.c_str(), selected)) {
			selected3D_ = object;
			selected2D_.reset();
		}
	}

	if (Is2D(object)) {
		bool selected = (selected2D_ == object);
		std::string label = name + "##2D" + std::to_string(object);
		if (ImGui::Selectable(label.c_str(), selected)) {

			selected2D_ = object;
			selected3D_.reset();
		}
	}
}

void ImGuiObjectEditor::CreateGroup() {

	groups_ = tagSystem_->Groups();
}

void ImGuiObjectEditor::SelectGroupedObject() {

	ImGui::SetWindowFontScale(0.84f);

	for (auto& [group, ids] : groups_) {
		if (group.empty()) continue;

		if (ImGui::TreeNode(group.c_str())) {
			for (uint32_t id : ids) {

				const std::string& name = tagSystem_->Tags().at(id)->name;
				DrawSelectable(id, name);
			}
			ImGui::TreePop();
		}
	}
	ImGui::SetWindowFontScale(1.0f);
}

void ImGuiObjectEditor::EditObject() {

	// 各objectの操作
	EditObjects();
	EditObject2D();
}

void ImGuiObjectEditor::Reset() {

	selected3D_.reset();
	selected2D_.reset();
}

void ImGuiObjectEditor::Registerobject(uint32_t id, IGameObject* object) {

	objectsMap_[id] = object;
}

void ImGuiObjectEditor::DrawManipulateGizmo(const GizmoContext& context) {

	// 描画先の設定
	ImGuizmo::SetDrawlist(context.drawlist);
	ImGuizmo::SetRect(context.rectMin.x, context.rectMin.y, context.rectSize.x, context.rectSize.y);
	ImGuizmo::SetOrthographic(context.orthographic);

	// 操作対象を決定
	uint32_t id = 0;
	bool is3D = false;
	bool is2D = false;
	if (selected3D_) {
		id = *selected3D_;
		is3D = true;

		// nullチェック
		if (!objectManager_->GetData<Transform3D>(id)) {
			return;
		}
	} else if (selected2D_) {
		id = *selected2D_;
		is2D = true;

		// nullチェック
		if (!objectManager_->GetData<Transform2D>(id)) {
			return;
		}
	} else {
		return;
	}

	// ワールド行列をcolumn-majorに
	float model[16];
	if (is3D) {

		auto* transform = objectManager_->GetData<Transform3D>(id);
		Math::ToColumnMajor(Matrix4x4::Transpose(transform->matrix.world), model);
	} else {

		auto* transform = objectManager_->GetData<Transform2D>(id);
		Math::ToColumnMajor(transform->matrix, model);
	}

	// 操作モード
	ImGuizmo::OPERATION option = currentOption_; // TRANSLATE / ROTATE / SCALE
	ImGuizmo::MODE mode = currentMode_;          // WORLD / LOCAL
	float snap[3] = { snapMove_, snapRotate_, snapScale_ };

	// 実行
	isUsingGuizmo_ = ImGuizmo::Manipulate(context.view, context.projection, option, mode, model,
		nullptr, useSnap_ ? snap : nullptr);

	// 変更があればSRTを書き戻す
	if (isUsingGuizmo_ && ImGuizmo::IsUsing()) {

		float scale[3]{};
		float rotate[3]{};
		float translate[3]{};
		ImGuizmo::DecomposeMatrixToComponents(model, translate, rotate, scale);
		if (is3D) {

			auto* transform = objectManager_->GetData<Transform3D>(id);
			transform->translation = Vector3(translate[0], translate[1], translate[2]);
			transform->eulerRotate = Vector3(rotate[0] * radian, rotate[1] * radian, rotate[2] * radian);
			transform->rotation = Quaternion::Normalize(Quaternion::EulerToQuaternion(transform->eulerRotate));
			transform->scale = Vector3(scale[0], scale[1], scale[2]);
			transform->SetIsDirty(true);
		} else {

			auto* transform = objectManager_->GetData<Transform2D>(id);
			transform->translation = Vector2(translate[0], translate[1]);
			transform->rotation = rotate[2];
			transform->size = Vector2(scale[0], scale[1]);
		}
	}

	// Guizmo状にマウスがあるかどうか
	isUsingGuizmo_ |= ImGuizmo::IsOver();
}

void ImGuiObjectEditor::GizmoToolbar() {

	ImGui::SeparatorText("Gizmo");
	if (ImGui::RadioButton("Translate", currentOption_ == ImGuizmo::TRANSLATE)) {
		currentOption_ = ImGuizmo::TRANSLATE;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", currentOption_ == ImGuizmo::ROTATE)) {
		currentOption_ = ImGuizmo::ROTATE;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Scale", currentOption_ == ImGuizmo::SCALE)) {
		currentOption_ = ImGuizmo::SCALE;
	}

	if (ImGui::RadioButton("World", currentMode_ == ImGuizmo::WORLD)) {
		currentMode_ = ImGuizmo::WORLD;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Local", currentMode_ == ImGuizmo::LOCAL)) {
		currentMode_ = ImGuizmo::LOCAL;
	}

	ImGui::Checkbox("Snap", &useSnap_);
	if (useSnap_) {

		ImGui::DragFloat("Move##Snap", &snapMove_, 0.01f);
		ImGui::DragFloat("Rotate##Snap", &snapRotate_, 0.5f * radian);
		ImGui::DragFloat("Scale##Snap", &snapScale_, 0.01f);
	}
}

void ImGuiObjectEditor::EditObjects() {

	if (!selected3D_) {
		return;
	}
	uint32_t id = selected3D_.value();
	if (!objectManager_->GetData<ObjectTag>(id)) {
		return;
	}
	const auto* tag = tagSystem_->Tags().at(id);
	if (tag->name != "skybox" && Algorithm::Find(objectsMap_, id)) {

		objectsMap_[id].value()->ImGui();
		return;
	}
	if (ImGui::BeginTabBar("Obj3DTab")) {

		// skyboxの時と他のオブジェクトで分岐
		if (tag->name == "skybox") {

			if (ImGui::BeginTabItem("Info")) {

				ObjectsInformation();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Skybox")) {

				EditSkybox();
				ImGui::EndTabItem();
			}
		} else {
			if (ImGui::BeginTabItem("Info")) {

				ObjectsInformation();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Transform")) {

				ObjectsTransform();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Material")) {

				ObjectsMaterial();
				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
	}
}

void ImGuiObjectEditor::ObjectsInformation() {

	uint32_t id = *selected3D_;
	const auto* tag = tagSystem_->Tags().at(id);

	ImGui::Text("name: %s", tag->name.c_str());
	ImGui::Text("objectId: %u", id);

	if (ImGui::Button("Remove")) {

		objectManager_->Destroy(id);
		selected3D_.reset();
	}
}

void ImGuiObjectEditor::ObjectsTransform() {

	auto* transform = objectManager_->GetData<Transform3D>(*selected3D_);
	transform->ImGui(itemWidth_);
}

void ImGuiObjectEditor::ObjectsMaterial() {

	auto* matsPtr = objectManager_->GetData<Material, true>(*selected3D_);
	auto& materials = *matsPtr;

	ImGui::PushItemWidth(itemWidth_);
	if (ImGui::BeginCombo("SelectMaterial",
		("Material " + std::to_string(selectedMaterialIndex_)).c_str())) {
		for (int i = 0; i < static_cast<int>(materials.size()); ++i) {

			bool selected = (selectedMaterialIndex_ == i);
			std::string label = "Material " + std::to_string(i);
			if (ImGui::Selectable(label.c_str(), selected)) {

				selectedMaterialIndex_ = i;
			}
			if (selected) {

				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();

	selectedMaterialIndex_ = std::clamp(selectedMaterialIndex_, 0, static_cast<int>(materials.size() - 1));
	if (!materials.empty()) {

		materials[selectedMaterialIndex_].ImGui(itemWidth_);
	}
}

void ImGuiObjectEditor::EditSkybox() {

	auto* skybox = objectManager_->GetData<Skybox>(*selected3D_);
	skybox->ImGui(itemWidth_);
}

void ImGuiObjectEditor::EditObject2D() {

	if (!selected2D_) return;
	uint32_t id = selected2D_.value();

	if (ImGui::BeginTabBar("Obj2DTab")) {

		if (ImGui::BeginTabItem("Info")) {

			Object2DInformation();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Sprite")) {

			Object2DSprite();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Transform")) {

			Object2DTransform();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Material")) {

			Object2DMaterial();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Derived")) {
			if (Algorithm::Find(objectsMap_, id)) {

				objectsMap_[id].value()->DerivedImGui();
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void ImGuiObjectEditor::Object2DInformation() {

	uint32_t id = *selected2D_;
	const auto* tag = tagSystem_->Tags().at(id);

	ImGui::Text("name: %s", tag->name.c_str());
	ImGui::Text("objectId: %u", id);

	if (ImGui::Button("Remove")) {

		objectManager_->Destroy(id);
		selected2D_.reset();
	}
}

void ImGuiObjectEditor::Object2DSprite() {

	auto* sprite = objectManager_->GetData<Sprite>(*selected2D_);
	sprite->ImGui(itemWidth_);
}

void ImGuiObjectEditor::Object2DTransform() {

	auto* transform = objectManager_->GetData<Transform2D>(*selected2D_);
	transform->ImGui(itemWidth_);
}

void ImGuiObjectEditor::Object2DMaterial() {

	auto* material = objectManager_->GetData<SpriteMaterial>(*selected2D_);
	material->ImGui(itemWidth_);
}