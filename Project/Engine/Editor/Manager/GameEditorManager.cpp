#include "GameEditorManager.h"

//============================================================================
//	include
//============================================================================

//============================================================================
//	GameEditorManager classMethods
//============================================================================

GameEditorManager* GameEditorManager::instance_ = nullptr;

GameEditorManager* GameEditorManager::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new GameEditorManager();
	}
	return instance_;
}

void GameEditorManager::Finalize() {

	if (instance_ != nullptr) {

		delete instance_;
		instance_ = nullptr;
	}
}

void GameEditorManager::AddEditor(IGameEditor* editor) {

	// editor追加
	editors_.emplace_back(editor);
}

void GameEditorManager::RemoveEditor(IGameEditor* editor) {

	// 選択中のeditorを削除したとき
	if (selectedEditor_) {
		if (selectedEditor_->GetName() == editor->GetName()) {

			selectedEditor_ = nullptr;
			selectedIndex_ = std::nullopt;
		}
	}

	// editor削除
	editors_.erase(std::remove(editors_.begin(), editors_.end(), editor), editors_.end());
}

void GameEditorManager::SetSelectObjectID(uint32_t id) {

	// 選択されていなければ早期リターン
	if (!selectedIndex_.has_value()) {
		return;
	}
	editors_[*selectedIndex_]->SetSelectObjectID(id);
}

void GameEditorManager::SelectEditor() {

	ImGui::SetWindowFontScale(0.84f);

	// editorsの要素を表示
	if (!editors_.empty()) {
		for (uint32_t index = 0; index < editors_.size(); ++index) {
			if (ImGui::Selectable(editors_[index]->GetName().c_str(),
				selectedIndex_.has_value() && selectedIndex_.value() == index)) {

				selectedIndex_ = index;
				selectedEditor_ = editors_[index];
			}
		}
	}
	ImGui::SetWindowFontScale(1.0f);
}

void GameEditorManager::EditEditor() {

	// 選択されていなければ早期リターン
	if (!selectedIndex_.has_value()) {
		return;
	}

	// 名前表示
	ImGui::Text("name: %s", editors_[*selectedIndex_]->GetName().c_str());

	ImGui::Separator();

	editors_[*selectedIndex_]->ImGui();
}