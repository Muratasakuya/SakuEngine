#include "GameObjectEditor.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/AssetEditor.h>
#include <Engine/Asset/Asset.h>
#include <Engine/Utility/ImGuiHelper.h>
#include <Engine/Object/Core/ObjectManager.h>

//============================================================================
//	GameObjectEditor classMethods
//============================================================================

void GameObjectEditor::Init(Asset* asset) {

	// layout
	leftChildSize_ = ImVec2(320.0f, 320.0f);
	rightChildSize_ = ImVec2(384.0f, 320.0f);
	addButtonSize_ = ImVec2(208.0f, 30.0f);
	dropSize_ = ImVec2(208.0f, 30.0f);

	ObjectManager_ = nullptr;
	ObjectManager_ = ObjectManager::GetInstance();

	asset_ = nullptr;
	asset_ = asset;
}

void GameObjectEditor::Update() {

	// 全てのobjectを更新
	UpdateEntities();
}

void GameObjectEditor::ImGui() {

	EditLayout();

	// layout
	ImGui::BeginGroup();

	// 追加処理
	ImGui::BeginChild("AddChild##GameObjectEditor", leftChildSize_, true);
	ImGui::SeparatorText("Add object");

	Addobject();
	ImGui::EndChild();

	// 横並びにする
	ImGui::SameLine();

	// 選択処理
	ImGui::BeginChild("SelectChild##GameObjectEditor", rightChildSize_, true);
	ImGui::SeparatorText("Select object");

	Selectobject();
	ImGui::EndChild();

	ImGui::BeginChild("EditChild##GameObjectEditor");
	ImGui::SeparatorText("Edit object");

	// 削除処理
	Removeobject();

	// 操作処理
	Editobject();
	ImGui::EndChild();

	ImGui::EndGroup();
}

void GameObjectEditor::EditLayout() {

	ImGui::Begin("GameObjectEditor_EditLayout");

	ImGui::DragFloat2("leftChildSize", &leftChildSize_.x, 0.5f);
	ImGui::DragFloat2("rightChildSize", &rightChildSize_.x, 0.5f);
	ImGui::DragFloat2("addButtonSize", &addButtonSize_.x, 0.5f);

	ImGui::End();
}

void GameObjectEditor::UpdateEntities() {

	// objectが存在しなければ処理しない
	if (entities_.empty()) {
		return;
	}

	for (const auto& entities : std::views::values(entities_)) {
		for (const auto& object : entities) {

			object->Update();
		}
	}
}

void GameObjectEditor::Addobject() {

	ImGui::PushID("AddobjectRow");

	if (ImGui::InputTextWithHint("##objectName", "objectName",
		addNameInputText_.nameBuffer, IM_ARRAYSIZE(addNameInputText_.nameBuffer))) {

		// 名前を保持
		addNameInputText_.name = addNameInputText_.nameBuffer;
	}

	// modelの名前
	DropFile("Drop model", addModelName_);
	// animationの名前
	DropFile("Drop animation", addAnimationName_);
	// typeの選択
	SelectobjectClassType(addClassType_);

	if (ImGui::Button("Add", addButtonSize_)) {

		// 入力がないときは追加不可
		if (addModelName_.has_value() &&
			!addNameInputText_.name.empty()) {

			// animationを読み込んでおく
			if (addAnimationName_.has_value()) {

				asset_->LoadAnimation(addAnimationName_.value(), addModelName_.value());
			}

			// object作成
			std::unique_ptr<GameObject3D>& enity = entities_[addClassType_].emplace_back(std::make_unique<GameObject3D>());
			enity->Init(addModelName_.value(), addNameInputText_.name,
				"GameObjectEditor", addAnimationName_);

			// 実際に追加された名前を取得
			const std::string name = enity->GetTag().name;

			// emitterの名前追加
			objectNames_.emplace_back(name);
			objectHandles_.push_back(objectHandle(addClassType_, entities_[addClassType_].size() - 1));
		}
	}

	ImGui::PopID();
}

void GameObjectEditor::Selectobject() {

	if (objectNames_.empty()) {
		ImGui::TextDisabled("object empty...");
		return;
	}

	for (int i = 0; i < static_cast<int>(objectNames_.size()); ++i) {

		const bool selected = (currentSelectIndex_.has_value() && currentSelectIndex_.value() == i);
		if (ImGui::Selectable(objectNames_[i].c_str(), selected)) {

			// indexで名前を選択し設定
			currentSelectIndex_ = i;
			selectobjectName_ = objectNames_[i];
		}
		if (selected) {
			ImGui::SetItemDefaultFocus();
		}
	}
}

void GameObjectEditor::Editobject() {

	// 選択されていないときは何もしない
	if (!currentSelectIndex_.has_value()) {
		return;
	}

	// object操作
	const objectHandle& handle = objectHandles_[currentSelectIndex_.value()];
	GameObject3D* object = entities_.at(handle.classType)[handle.innerIndex].get();
	object->ImGui();
}

void GameObjectEditor::Removeobject() {

	// 選択されていないときは何もしない
	if (!currentSelectIndex_.has_value()) {
		return;
	}

	if (ImGui::Button("Remove object", addButtonSize_)) {

		// 現在のインデックスを取得
		const int listIndex = currentSelectIndex_.value();
		const objectHandle handle = objectHandles_[listIndex];

		// map内の選択されているobjectを削除する
		auto mapIt = entities_.find(handle.classType);
		if (mapIt != entities_.end() && handle.innerIndex < mapIt->second.size()) {

			mapIt->second.erase(mapIt->second.begin() + static_cast<std::ptrdiff_t>(handle.innerIndex));

			// innerIndexも削除する
			for (auto& h : objectHandles_) {
				if (h.classType == handle.classType && h.innerIndex > handle.innerIndex) {
					--h.innerIndex;
				}
			}
		}

		objectNames_.erase(objectNames_.begin() + listIndex);
		objectHandles_.erase(objectHandles_.begin() + listIndex);

		if (objectNames_.empty()) {

			currentSelectIndex_.reset();
			selectobjectName_.reset();
		} else {

			const int newIndex = std::min(listIndex, static_cast<int>(objectNames_.size() - 1));
			currentSelectIndex_ = newIndex;
			selectobjectName_ = objectNames_[newIndex];
		}
	}
}

void GameObjectEditor::DropFile(const std::string& label, std::optional<std::string>& recieveName) {

	// ドロップ処理
	// まだ設定されていないときのみ
	if (!recieveName.has_value()) {

		// ドラッグアンドドロップで取得
		ImGui::Button(label.c_str(), dropSize_);

		if (ImGui::BeginDragDropTarget()) {
			if (const auto* payload = ImGuiHelper::DragDropPayload(PendingType::Model)) {

				// 名前を保存
				recieveName = std::string(payload->name);
			}
			ImGui::EndDragDropTarget();
		}
	} else {

		if (ImGui::Button(("Reset##" + label).c_str(), ImVec2(addButtonSize_.x / 2.0f, addButtonSize_.y))) {

			// 名前をリセットする
			recieveName = std::nullopt;
		}
		ImGui::SameLine();

		if (recieveName.has_value()) {

			// 名を表示
			ImGui::Text((":" + recieveName.value()).c_str());
		}
	}
}

void GameObjectEditor::SelectobjectClassType(objectClassType& classType) {

	const char* typeOptions[] = {
			"None",
	};

	int typeIndex = static_cast<int>(classType);
	if (ImGui::BeginCombo("Type", typeOptions[typeIndex])) {
		for (int i = 0; i < IM_ARRAYSIZE(typeOptions); i++) {

			const bool isSelected = (typeIndex == i);
			if (ImGui::Selectable(typeOptions[i], isSelected)) {

				typeIndex = i;
				classType = static_cast<objectClassType>(i);
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
}

void GameObjectEditor::InputTextValue::Reset() {

	// 入力をリセット
	nameBuffer[0] = '\0';
	name.clear();
}