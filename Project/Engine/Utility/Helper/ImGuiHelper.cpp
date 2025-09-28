#include "ImGuiHelper.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Window/WinApp.h>
#include <Engine/Asset/AssetEditor.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/System/Systems/TagSystem.h>
#include <Engine/Object/Data/Transform.h>
#include <Engine/Utility/Json/JsonAdapter.h>

// windows
#include <commdlg.h>
#include <windows.h>

//============================================================================
//	ImGuiHelper classMethods
//============================================================================

void ImGuiHelper::ImageButtonWithLabel(const char* id,
	const std::string& label, ImTextureID textureId, const ImVec2& size) {

	ImGui::PushID(id);
	ImGui::BeginGroup();

	// テキストサイズ計算
	ImVec2 textSize = ImGui::CalcTextSize(label.c_str());

	// 現在の位置取得
	ImVec2 pos = ImGui::GetCursorScreenPos();

	// 上にテキストの高さ分スペースを確保
	float labelSpacing = 2.0f;
	ImGui::Dummy(ImVec2(size.x, textSize.y + labelSpacing));

	// 画像ボタンの描画
	ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + textSize.y + labelSpacing));
	ImGui::ImageButton(label.c_str(), textureId, size);

	// テキストをボタンの上に中央揃えで描画
	ImVec2 textPos = ImVec2(
		pos.x + (size.x - textSize.x) * 0.5f,
		pos.y);
	ImGui::GetWindowDrawList()->AddText(
		textPos,
		ImGui::GetColorU32(ImGuiCol_Text),
		label.c_str());

	ImGui::EndGroup();
	ImGui::PopID();
}

const DragPayload* ImGuiHelper::DragDropPayload(PendingType expectedType) {

	if (!ImGui::BeginDragDropTarget()) {
		return nullptr;
	}

	const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kDragPayloadId);
	if (payload) {
		auto* dragPayload = static_cast<const DragPayload*>(payload->Data);
		if (dragPayload && dragPayload->type == expectedType) {

			ImGui::EndDragDropTarget();
			return dragPayload;
		}
	}

	ImGui::EndDragDropTarget();
	return nullptr;
}

bool ImGuiHelper::ComboFromStrings(const char* label, int* currentIndex,
	const std::vector<std::string>& items, int popupMaxHeightInItems) {

	std::vector<const char*> itemNames;
	itemNames.reserve(items.size());
	for (auto& item : items) {

		itemNames.push_back(item.c_str());
	}
	if (itemNames.empty()) {

		bool changed = ImGui::Combo(label, currentIndex, nullptr, 0, popupMaxHeightInItems);
		return changed;
	}

	int before = *currentIndex;
	bool changed = ImGui::Combo(label, currentIndex, itemNames.data(),
		static_cast<int>(itemNames.size()), popupMaxHeightInItems);
	return changed || (before != *currentIndex);
}

bool ImGuiHelper::SelectableListFromStrings(const char* label, int* currentIndex,
	const std::vector<std::string>& items, int heightInItems) {

	if (label && *label) {

		ImGui::TextUnformatted(label);
	}
	// 表示用の高さを項目数に合わせて計算
	const int n = static_cast<int>(items.size());
	const int rows = (heightInItems > 0) ? heightInItems : n;
	float itemHeight = ImGui::GetTextLineHeightWithSpacing();
	ImVec2 listSize(ImGui::GetContentRegionAvail().x, itemHeight * rows + ImGui::GetStyle().FramePadding.y * 2.0f);

	// 内部スクロール付きの子ウィンドウで並べる
	ImGui::BeginChild(ImGui::GetID(label), listSize, true, ImGuiWindowFlags_HorizontalScrollbar);
	bool changed = false;

	for (int i = 0; i < n; ++i) {

		bool selected = (*currentIndex == i);
		if (ImGui::Selectable(items[i].c_str(), selected)) {
			if (*currentIndex != i) {

				*currentIndex = i;
				changed = true;
			}
		}
	}
	ImGui::EndChild();
	return changed;
}

bool ImGuiHelper::BeginFramedChild(const char* id, const char* title,
	const ImVec2& size, ImGuiWindowFlags flags) {

	if (title && *title) {

		ImGui::SeparatorText(title);
	}
	return ImGui::BeginChild(id, size, true, flags);
}

void ImGuiHelper::EndFramedChild() {

	ImGui::EndChild();
}

bool ImGuiHelper::SelectTagTarget(const char* label, uint32_t* ioSelectedId,
	std::string* outName, const char* groupFilter) {

	ObjectManager* objectManager = ObjectManager::GetInstance();
	TagSystem* tagSystem = objectManager->GetSystem<TagSystem>();
	const auto& groups = tagSystem->Groups();
	const auto& tags = tagSystem->Tags();

	if (label && *label) {

		ImGui::TextUnformatted(label);
	}
	bool changed = false;
	for (const auto& [group, ids] : groups) {

		if (group.empty()) {
			continue;
		}
		if (groupFilter && *groupFilter && group != groupFilter) {
			continue;
		}
		if (ImGui::TreeNode(group.c_str())) {
			for (uint32_t id : ids) {

				// 追従先を設定できるオブジェクトのみ
				if (!objectManager->GetData<Transform3D>(id)) {
					continue;
				}

				const std::string& name = tags.at(id)->name;
				bool isSel = (ioSelectedId && *ioSelectedId == id);
				std::string labelId = name + "##" + std::to_string(id);
				if (ImGui::Selectable(labelId.c_str(), isSel)) {
					if (ioSelectedId) {
						*ioSelectedId = id;
					}
					if (outName) {
						*outName = name;
					}
					changed = true;
				}
			}
			ImGui::TreePop();
		}
	}
	return changed;
}

bool ImGuiHelper::SaveJsonModal(const char* popupTitle, const char* baseDirLabelEx,
	const char* prefixOnSave, JsonSaveState& ioState, std::string& outRelPath) {

	if (ioState.showPopup) {
		ImGui::OpenPopup(popupTitle);
	}

	bool decided = false;
	if (ImGui::BeginPopupModal(popupTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

		// 保存先の表示
		ImGui::Text("%s%s", baseDirLabelEx ? baseDirLabelEx : "", ioState.input);

		// 入力テキスト
		ImGui::InputText("##JsonFilename", ioState.input, JsonSaveState::kBuffer);

		// Save ボタン
		if (ImGui::Button("Save")) {

			std::string input = ioState.input;
			if (!input.empty()) {

				outRelPath = std::string(prefixOnSave ? prefixOnSave : "") + input;
				decided = true;
				ioState.showPopup = false;
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {

			ioState.showPopup = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	return decided;
}

bool ImGuiHelper::OpenJsonDialog(std::string& outRelPath) {

	char szFile[MAX_PATH] = {};
	OPENFILENAMEA ofn{};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = WinApp::GetHwnd();
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = "JSON (*.json)\0*.json\0All\0*.*\0";

	static const std::string kInitDir =
		std::filesystem::absolute(JsonAdapter::baseDirectoryFilePath_).string();
	ofn.lpstrInitialDir = kInitDir.c_str();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameA(&ofn)) {

		namespace fs = std::filesystem;
		fs::path base = fs::weakly_canonical(JsonAdapter::baseDirectoryFilePath_);
		fs::path full = fs::weakly_canonical(ofn.lpstrFile);
		try {
			outRelPath = fs::relative(full, base).generic_string();
		}
		catch (...) {
			outRelPath = full.filename().generic_string();
		}
		return true;
	}
	return false;
}