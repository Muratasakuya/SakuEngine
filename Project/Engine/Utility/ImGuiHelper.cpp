#include "ImGuiHelper.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/AssetEditor.h>

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