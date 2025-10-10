#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/AssetEditorPayloadData.h>
#include <Engine/MathLib/MathUtils.h>

// c++
#include <string>
#include <string_view>
#include <vector>
#include <format>
#include <type_traits>
#include <filesystem>
// imgui
#include <imgui.h>

//============================================================================
//	ImGuiHelper structures
//============================================================================

namespace detail {

	template<class T>
	concept Arithmetic = std::is_arithmetic_v<T>;

	template<class T>
	concept HasXY = requires(const T & v) { v.x; v.y; };
	template<class T>
	concept HasXYZ = requires(const T & v) { v.x; v.y; v.z; };

	template<Arithmetic T>
	inline std::string format_value(const T& v, int precision) {
		if constexpr (std::is_floating_point_v<T>) {

			return std::format("{:.{}f}", v, precision);
		} else {

			return std::format("{}", v);
		}
	}

	inline std::string format_value(const Vector2& v, int precision) {

		return std::format("({:.{}f}, {:.{}f})", v.x, precision, v.y, precision);
	}
	inline std::string format_value(const Vector3& v, int precision) {

		return std::format("({:.{}f}, {:.{}f}, {:.{}f})",
			v.x, precision, v.y, precision, v.z, precision);
	}
}

struct JsonSaveState {

	bool showPopup = false;
	static constexpr int kBuffer = 128;
	char input[kBuffer] = {};
};

//============================================================================
//	ImGuiHelper class
//============================================================================
class ImGuiHelper {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ImGuiHelper() = default;
	~ImGuiHelper() = default;

	static void ImageButtonWithLabel(const char* id,
		const std::string& label, ImTextureID textureId, const ImVec2& size);

	static const DragPayload* DragDropPayload(PendingType expectedType);
	static std::string DragDropPayloadString(PendingType expectedType);

	// 配列のstringをComboで表示する
	static bool ComboFromStrings(const char* label, int* currentIndex,
		const std::vector<std::string>& items, int popupMaxHeightInItems = -1);
	template <typename T>
	static bool ComboFromKeys(const char* label, int* currentIndex,
		const T& container, std::string* outSelectedKey = nullptr,
		int popupMaxHeightInItems = -1);
	// 配列のstringをSelectableで表示する
	static bool SelectableListFromStrings(const char* label, int* currentIndex,
		const std::vector<std::string>& items, int heightInItems = 8);
	template <typename T>
	static bool SelectableListFromKeys(const char* label, int* currentIndex,
		const T& container, std::string* outSelectedKey = nullptr, int heightInItems = 8);

	// 枠
	static bool BeginFramedChild(const char* id, const char* title,
		const ImVec2& size, ImGuiWindowFlags flags = 0);
	static void EndFramedChild();

	// TagSystemから名前の取得をする
	static bool SelectTagTarget(const char* label, uint32_t* ioSelectedId,
		std::string* outName = nullptr, const char* groupFilter = nullptr);

	// 値の表示
	template <typename T>
	static void ValueText(const char* label, const T& value, int precision = 3);

	// jsonの保存
	static bool SaveJsonModal(const char* popupTitle, const char* baseDirLabelEx,
		const char* prefixOnSave, JsonSaveState& ioState, std::string& outRelPath);
	// jsonの読み込みファイルダイアログ
	static bool OpenJsonDialog(std::string& outRelPath);

	// ポストプロセスのマスクの設定
	static bool EditPostProcessMask(uint32_t& ioMask);
};

//============================================================================
//	ImGuiHelper templateMethods
//============================================================================

template<typename T>
inline bool ImGuiHelper::ComboFromKeys(const char* label, int* currentIndex,
	const T& container, std::string* outSelectedKey, int popupMaxHeightInItems) {

	std::vector<const char*> itemNames;
	std::vector<const std::string*> refs;
	itemNames.reserve(container.size());
	refs.reserve(container.size());
	for (const auto& item : container) {

		refs.push_back(&item.first);
		itemNames.push_back(item.first.c_str());
	}
	if (itemNames.empty()) {

		return ImGui::Combo(label, currentIndex, nullptr, 0, popupMaxHeightInItems);
	}

	int before = *currentIndex;
	bool changed = ImGui::Combo(label, currentIndex, itemNames.data(),
		static_cast<int>(itemNames.size()), popupMaxHeightInItems);
	if (outSelectedKey && *currentIndex >= 0 && *currentIndex < static_cast<int>(refs.size())) {

		*outSelectedKey = *refs[*currentIndex];
	}
	return changed || (before != *currentIndex);
}

template<typename T>
inline bool ImGuiHelper::SelectableListFromKeys(const char* label, int* currentIndex,
	const T& container, std::string* outSelectedKey, int heightInItems) {

	std::vector<std::string> keys;
	keys.reserve(container.size());
	for (const auto& item : container) {

		keys.push_back(item.first);
	}
	const bool changed = SelectableListFromStrings(label, currentIndex, keys, heightInItems);
	if (changed && outSelectedKey && *currentIndex >= 0 && *currentIndex < static_cast<int>(keys.size())) {

		*outSelectedKey = keys[*currentIndex];
	}
	return changed;
}

template<typename T>
inline void ImGuiHelper::ValueText(const char* label, const T& value, int precision) {

	using detail::format_value;
	const std::string stringValue = format_value(value, precision);
	ImGui::TextUnformatted(std::format("{}: {}", label, stringValue).c_str());
}