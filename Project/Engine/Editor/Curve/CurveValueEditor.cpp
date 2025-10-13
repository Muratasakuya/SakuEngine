#include "CurveValueEditor.h"

//============================================================================
//	include
//============================================================================

// imgui
#include <imgui.h>

//============================================================================
//	CurveValueEditor classMethods
//============================================================================

CurveValueEditor* CurveValueEditor::instance_ = nullptr;

CurveValueEditor* CurveValueEditor::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new CurveValueEditor();
	}
	return instance_;
}

void CurveValueEditor::Finalize() {

	if (instance_ != nullptr) {

		delete instance_;
		instance_ = nullptr;
	}
}

void CurveValueEditor::Registry(void* key, std::function<bool()> draw) {

	int index = FindIndex(key);
	// 新しいカーブの登録
	if (index < 0) {

		Entry entry{};
		entry.key = key;
		entry.draw = std::move(draw);
		entries_.push_back(std::move(entry));
	}
	// すでに登録済みのカーブ
	else {
		if (draw) {

			entries_[index].draw = std::move(draw);
		}
	}
}

void CurveValueEditor::Open(void* key) {

	// キーからインデックスがあればエディターを開く
	int index = FindIndex(key);
	if (0 <= index) {

		activeIndex_ = index;
		showWindow_ = true;
	}
}

bool CurveValueEditor::ConsumeChanged(void* key) {

	int index = FindIndex(key);
	if (index < 0) {

		return false;
	}
	bool changed = entries_[index].changed;
	entries_[index].changed = false;
	return changed;
}

void CurveValueEditor::Edit() {

	if (!showWindow_) {
		return;
	}

	if (0 <= activeIndex_ && activeIndex_ < static_cast<int>(entries_.size())) {

		Entry& entry = entries_[activeIndex_];
		if (entry.draw) {

			bool changedFrame = entry.draw();
			entry.changed |= changedFrame;
		}
	} else {

		ImGui::TextDisabled("No curve selected.");
	}

	if (!showWindow_) activeIndex_ = -1;

}

int CurveValueEditor::FindIndex(void* key) const {

	for (int i = 0; i < static_cast<int>(entries_.size()); ++i) {
		// 既に登録済み
		if (entries_[i].key == key) {

			return i;
		}
	}
	// 新しいキーの登録
	return -1;
}