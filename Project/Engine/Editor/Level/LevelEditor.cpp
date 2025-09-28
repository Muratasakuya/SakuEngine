#include "LevelEditor.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	LevelEditor classMethods
//============================================================================

void LevelEditor::Init(const std::string& initSceneFile) {

	sceneBuilder_ = std::make_unique<SceneBuilder>();
	sceneBuilder_->Init(jsonPath_);

	sceneBuilder_->SetFile(initSceneFile);

	// objectの作成
	BuildObjects();

	rightChildSize_ = ImVec2(384.0f, 320.0f);
	buttonSize_ = ImVec2(256.0f, 32.0f);
}

void LevelEditor::SaveObject(GameObject3D* object) {

	const std::string& identifier = Algorithm::RemoveAfterUnderscore(object->GetIdentifier());
	if (ImGui::Button(("Save Material..." + identifier + ".json").c_str())) {

		Json data;
		// materialを保存
		object->SaveMaterial(data);

		JsonAdapter::Save(jsonPath_ + identifier + ".json", data);
	}
}

void LevelEditor::Update() {

	// objectの作成
	BuildObjects();

	// objectの更新処理
	UpdateObjects();
}

void LevelEditor::BuildObjects() {

	// sceneを構築するかどうか
	if (sceneBuilder_->IsCreate()) {

		// .jsonを基に作成
		sceneBuilder_->CreateObjectsMap(objectsMap_);
		// リセット
		sceneBuilder_->Reset();
	}
}

void LevelEditor::UpdateObjects() {

	if (objectsMap_.empty()) {
		return;
	}

	// 種類ごとに全て更新
	for (const auto& objectMap : std::views::values(objectsMap_)) {
		for (const auto& entities : objectMap) {

			entities->Update();
		}
	}
}

void LevelEditor::ImGui() {

	sceneBuilder_->ImGui();

	// 選択処理
	ImGui::BeginChild("SelectChild##SceneBuilder", rightChildSize_, true);
	ImGui::SeparatorText("Select object");

	SelectObject();
	ImGui::EndChild();

	ImGui::BeginChild("EditChild##SceneBuilder");
	ImGui::SeparatorText("Edit object");

	// 操作処理
	EditObject();
	ImGui::EndChild();

	ImGui::EndGroup();
}

void LevelEditor::SelectObject() {

	// objectType選択
	const char* typeOptions[] = { 
		"None","CrossMarkWall"
	};

	int typeIndex = static_cast<int>(currentSelectType_);
	if (ImGui::BeginCombo("Type", typeOptions[typeIndex])) {
		for (int i = 0; i < IM_ARRAYSIZE(typeOptions); ++i) {

			bool isSelected = (typeIndex == i);
			if (ImGui::Selectable(typeOptions[i], isSelected)) {

				typeIndex = i;
				currentSelectType_ = static_cast<Level::ObjectType>(i);
				currentSelectIndex_.reset();
				selectFilter_.Clear();
			}
			if (isSelected) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	auto mapIt = objectsMap_.find(currentSelectType_);
	if (mapIt == objectsMap_.end() || mapIt->second.empty()) {
		ImGui::TextDisabled("No object for this type...");
		return;
	}

	// 検索ボックス 
	selectFilter_.Draw("##Searchobject", rightChildSize_.x - 16.0f);
	ImGui::Separator();

	// 一覧
	const auto& objectVec = mapIt->second;
	bool hasResult = false;
	for (int i = 0; i < static_cast<int>(objectVec.size()); ++i) {

		const std::string name = objectVec[i]->GetTag().name + "_" + objectVec[i]->GetIdentifier();
		if (!selectFilter_.PassFilter(name.c_str())) { 
			continue;
		}

		hasResult = true;
		bool selected = (currentSelectIndex_ && *currentSelectIndex_ == i);
		if (ImGui::Selectable(name.c_str(), selected)) {

			currentSelectIndex_ = i;
		}
		if (selected) ImGui::SetItemDefaultFocus();
	}

	// 検索に引っかからなかった場合
	if (!hasResult) {

		ImGui::TextDisabled("No matches...");
	}
}

void LevelEditor::EditObject() {

	if (!currentSelectIndex_.has_value()) {
		return;
	}

	auto mapIt = objectsMap_.find(currentSelectType_);
	if (mapIt == objectsMap_.end()) {
		return;
	}

	const auto& objectVec = mapIt->second;
	int index = currentSelectIndex_.value();
	if (index < 0 || index >= static_cast<int>(objectVec.size())) {
		return;
	}

	SaveObject(objectVec[index].get());
	objectVec[index]->ImGui();
}