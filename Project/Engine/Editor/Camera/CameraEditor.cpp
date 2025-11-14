#include "CameraEditor.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/GameObject/ImGuiObjectEditor.h>
#include <Engine/Scene/SceneView.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
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

void CameraEditor::Update() {

	// キーオブジェクトの更新
	UpdateKeyObjects();

	// エディターの更新
	UpdateEditor();
}

void CameraEditor::UpdateKeyObjects() {

	// キーオブジェクトの更新
	for (auto& keyObject : std::views::values(keyObjects_)) {

		switch (previewMode_) {
		case CameraEditor::PreviewMode::Keyframe:

			break;
		case CameraEditor::PreviewMode::Manual:

			// 時間を渡して更新
			keyObject->ExternalInputTUpdate(previewTimer_);
			break;
		case CameraEditor::PreviewMode::Play:

			// KeyframeObject内で自己完結で更新
			keyObject->SelfUpdate();
			break;
		}
	}
}

void CameraEditor::UpdateEditor() {
#if defined(_DEBUG) || defined(_DEVELOPBUILD)

	// 現在のゲームカメラ
	BaseCamera* camera = sceneView_->GetCamera();
	// 更新設定
	camera->SetIsUpdateEditor(isPreViewGameCamera_);

	// プレビュー表示していないときはそもそも更新しない
	if (!isPreViewGameCamera_) {
		return;
	}
	// 存在しないキーでは処理させない
	if (!Algorithm::Find(keyObjects_, selectedKeyObjectName_)) {
		return;
	}

	// モード別の更新
	switch (previewMode_) {
	case CameraEditor::PreviewMode::Keyframe: {

		// IDの同期
		SynchSelectedKeyIndex();

		// 0以上の場合のみ
		if (previewKeyIndex_ < 0 || keyObjects_[selectedKeyObjectName_]->GetKeyObjectIDs().empty()) {
			break;
		}

		// 現在のキー位置のカメラ情報を取得して反映させる
		Transform3D transform = keyObjects_[selectedKeyObjectName_]->GetIndexTransform(previewKeyIndex_);
		float fovY = 0.0f;
		KeyframeObject3D::AnyValue fovValue = keyObjects_[selectedKeyObjectName_]->GetIndexAnyValue(previewKeyIndex_, addKeyValueFov_);
		if (const auto& keyFovY = std::get_if<float>(&fovValue)) {

			fovY = *keyFovY;
		}

		// 現在のキー位置のカメラ情報を渡す
		camera->SetTranslation(transform.translation);
		camera->SetRotation(transform.rotation);
		camera->SetFovY(fovY);

		// カメラの更新
		camera->UpdateView(BaseCamera::UpdateMode::Quaternion);
		break;
	}
	case CameraEditor::PreviewMode::Manual: {

		// カメラへ適応
		ApplyToCamera(*camera, selectedKeyObjectName_);
		break;
	}
	case CameraEditor::PreviewMode::Play: {

		// カメラへ適応
		ApplyToCamera(*camera, selectedKeyObjectName_);

		// 再生中は時間を更新しない
		if (keyObjects_[selectedKeyObjectName_]->IsUpdating()) {
			break;
		}

		// 経過時間を更新
		previewLoopTimer_ += GameTimer::GetDeltaTime();
		// 時間経過後再生
		if (previewLoopSpacing_ < previewLoopTimer_) {

			// 現在値操作のキーの再生
			keyObjects_[selectedKeyObjectName_]->StartLerp();
			// リセット
			previewLoopTimer_ = 0.0f;
		}
		break;
	}
	}
#endif
}

void CameraEditor::ApplyToCamera(BaseCamera& camera, const std::string& keyName) {

	// 現在のキー位置のカメラ情報
	Transform3D transform = keyObjects_[keyName]->GetCurrentTransform();

	float fovY = 0.0f;
	KeyframeObject3D::AnyValue fovValue = keyObjects_[keyName]->GetCurrentAnyValue(addKeyValueFov_);
	if (const auto& keyFovY = std::get_if<float>(&fovValue)) {

		fovY = *keyFovY;
	}

	// 現在のキー位置のカメラ情報を渡す
	camera.SetTranslation(transform.translation);
	camera.SetRotation(transform.rotation);
	camera.SetFovY(fovY);

	// カメラの更新
	camera.UpdateView(BaseCamera::UpdateMode::Quaternion);
}

void CameraEditor::SynchSelectedKeyIndex() {

	// 選択されているオブジェクトに合わせる
	const auto& selected = ImGuiObjectEditor::GetInstance()->GetSelected3D();
	// 未選択
	if (!selected.has_value()) {
		previewKeyIndex_ = -1;
		return;
	}

	// 同じIDを検索
	for (const auto& id : keyObjects_[selectedKeyObjectName_]->GetKeyObjectIDs()) {
		if (id == selected.value()) {

			//　選択されているオブジェクトが何番目のキーインデックスか取得して設定
			previewKeyIndex_ = keyObjects_[selectedKeyObjectName_]->GetKeyIndexFromObjectID(id);
		}
	}
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
				object->AddKeyValue(AnyMold::Float, addKeyValueFov_);
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

	ImGui::PushItemWidth(200.0f);

	//================================================================================================================
	//	キーオブジェクトの編集
	//================================================================================================================

	if (ImGui::BeginTabBar("CameraEditorTabBar")) {
		//================================================================================================================
		//	ゲームカメラとの連携
		//================================================================================================================
		if (ImGui::BeginTabItem("GameCamera")) {

			// モード選択
			ImGui::Checkbox("isPreViewGameCamera_", &isPreViewGameCamera_);
			EnumAdapter<PreviewMode>::Combo("PreviewMode", &previewMode_);

			ImGui::SeparatorText("Option");

			// モード別オプション
			switch (previewMode_) {
			case CameraEditor::PreviewMode::Keyframe: {

				// 選択中のキーのインデックスを取得してその位置のキー表示する

				break;
			}
			case CameraEditor::PreviewMode::Manual: {

				ImGui::DragFloat("previewTimer", &previewTimer_, 0.001f, 0.0f, 1.0f);
				break;
			}
			case CameraEditor::PreviewMode::Play: {

				ImGui::DragFloat("previewLoopSpacing", &previewLoopSpacing_, 0.01f);
				ImGui::Text("current: %.2f / %.2f", previewLoopTimer_, previewLoopSpacing_);
				break;
			}
			}
			ImGui::EndTabItem();
		}
		//================================================================================================================
		//	各キーの調整、it->second->ImGui()内では親子付けまでできる
		//================================================================================================================		
		if (ImGui::BeginTabItem("KeyObject")) {

			// keyframeObjectのImGui関数を呼びだす
			it->second->ImGui();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::PopItemWidth();
}