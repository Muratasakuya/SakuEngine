#include "CameraEditor.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Helper/ImGuiHelper.h>
#include <Engine/Utility/Helper/Algorithm.h>

//============================================================================
//	CameraEditor classMethods
//============================================================================

CameraEditor* CameraEditor::instance_ = nullptr;

CameraEditor* CameraEditor::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new CameraEditor();
	}
	return instance_;
}

void CameraEditor::Finalize() {

	if (instance_ != nullptr) {

		delete instance_;
		instance_ = nullptr;
	}
}

void CameraEditor::Init(SceneView* sceneView) {

	sceneView_ = nullptr;
	sceneView_ = sceneView;
}

void CameraEditor::ImGui() {

	// キーオブジェクトの追加、選択
	AddAndSelectKeyObjectMap();
	// キーオブジェクトの編集
	EditSelectedKeyObject();
}

void CameraEditor::AddAndSelectKeyObjectMap() {

	ImVec2 areaSize = ImGuiHelper::GetWindowAreaSizeRatio(0.5f, 0.5f);
	const float areaHeight = 128.0f;

	//================================================================================================================
	//	キーオブジェクトの追加
	//================================================================================================================

	// 左側の枠
	if (ImGuiHelper::BeginFramedChild("##Add", nullptr, ImVec2(areaSize.x, areaHeight))) {

		ImGui::TextUnformatted("Add Key");

		// 入力欄
		static std::string inputName;
		char buf[128] = {};
		strncpy_s(buf, sizeof(buf), inputName.c_str(), _TRUNCATE);
		if (ImGui::InputText("Name", buf, IM_ARRAYSIZE(buf))) {

			inputName = buf;
		}

		if (ImGui::Button("Add Key")) {

			// 既に存在しているキーの名前で追加できない
			if (!inputName.empty() && !Algorithm::Find(keyObjects_, inputName)) {

				// キーオブジェクトを生成
				std::unique_ptr<KeyframeObject3D> object = std::make_unique<KeyframeObject3D>();
				// fovYを任意の値として追加
				object->AddKeyValue(AnyMold::Float, "FovY");
				object->Init(keyObjectName_, keyModelName_);

				// 追加
				keyObjects_.emplace(inputName, std::move(object));

				// 選択を更新
				selectedKeyObjectName_ = inputName;
				inputName.clear();
			}
		}
	}
	ImGuiHelper::EndFramedChild();

	// 同じライン
	ImGui::SameLine();

	//================================================================================================================
	//	キーオブジェクトの選択
	//================================================================================================================

	// 右側の枠
	if (ImGuiHelper::BeginFramedChild("##Select", nullptr, ImVec2(areaSize.y, areaHeight))) {

		// キーオブジェクトの選択
		// stringを配列にまとめる
		std::vector<std::string> keyNames;
		for (const auto& key : keyObjects_) {

			keyNames.emplace_back(key.first);
		}

		// 現在のindexをkeysから逆引き
		int32_t currentIndex = -1;
		if (!selectedKeyObjectName_.empty()) {
			for (int i = 0; i < static_cast<int32_t>(keyObjects_.size()); ++i) {
				// 同じ名前のインデックスを探す
				if (keyNames[i] == selectedKeyObjectName_) {

					// インデックスを更新
					currentIndex = i;
					break;
				}
			}
		}

		// 何もない場合は文字を表示する
		if (keyNames.empty()) {

			ImGui::TextDisabled("Key is Empty");
		}
		// リスト選択
		else {
			if (ImGuiHelper::SelectableListFromStrings("CameraKey List", &currentIndex, keyNames, 32)) {
				// 選択されたキーオブジェクト名を更新
				if (currentIndex >= 0 && currentIndex < static_cast<int32_t>(keyNames.size())) {

					selectedKeyObjectName_ = keyNames[currentIndex];
				}
			}
		}
	}
	ImGuiHelper::EndFramedChild();
}

void CameraEditor::EditSelectedKeyObject() {

	// 未選択ならなにもしない
	if (selectedKeyObjectName_.empty()) {
		return;
	}
	// 存在しないキーなら処理しないし選択も解除
	auto it = keyObjects_.find(selectedKeyObjectName_);
	if (it == keyObjects_.end()) {
		selectedKeyObjectName_.clear();
		return;
	}

	//================================================================================================================
	//	キーオブジェクトの編集
	//================================================================================================================

	if (ImGui::BeginTabBar("CameraEditorTabBar")) {
		if (ImGui::BeginTabItem("KeyObject")) {

			// keyframeObjectのImGui関数を呼びだす
			it->second->ImGui();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}