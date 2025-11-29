#include "CameraEditor.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/GameObject/ImGuiObjectEditor.h>
#include <Engine/Core/Debug/SpdLogger.h>
#include <Engine/Scene/SceneView.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
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

void CameraEditor::LoadJson(const std::string& fileName, bool isInEditor) {

	LOG_SCOPE_MS_LABEL("CameraEditorLoadJson");

	// エディタからの呼び出しならfileNameに"CameraEditor/xxx.json"が入っているのでそのまま、
	// ゲーム側からなら sonBasePath_ 前にくっつける
	std::string jsonFileName = fileName;
	if (!isInEditor) {
		jsonFileName = jsonBasePath_ + fileName;
	}

	// Json読み込み
	Json data;
	if (!JsonAdapter::LoadCheck(jsonFileName, data)) {
		// 読み込めなければエラーにする
		ASSERT(FALSE, "failed to open file:" + jsonFileName);
		return;
	}

	// keyName、CameraKeyObjectを探索して見つからなければ早期リターン
	if (!data.contains("keyName") || !data.contains("CameraKeyObject")) {
		LOG_INFO("this file is not CameraKeyDataFile: [{}]", jsonFileName);
		return;
	}

	// キーの名前
	std::string keyName = data["keyName"];
	// 同じキーの場合
	if (Algorithm::Find(keyObjects_, keyName)) {

		// エディター読み込みの場合は名前に連番をつける
		if (isInEditor) {

			keyName = CheckName(keyName);
		}
		// ゲーム読み込みの場合は読み込まない
		else {
			return;
		}
	}

	// キーオブジェクト
	std::unique_ptr<KeyframeObject3D> keyObject = std::make_unique<KeyframeObject3D>();

	// 初期化
	keyObject->Init(keyObjectName_, keyModelName_);
	// 追加するキー情報
	keyObject->AddKeyValue(AnyMold::Float, addKeyValueFov_);
	// Jsonから復元
	keyObject->FromJson(data["CameraKeyObject"]);

	// 名前で追加
	keyObjects_.emplace(keyName, std::move(keyObject));

	// エディター
	previewMode_ = EnumAdapter<PreviewMode>::FromString(data["previewMode_"]).value();
	previewLoopSpacing_ = data.value("previewLoopSpacing_", 1.0f);

	LOG_INFO("loaded CameraKeyData: fileName: [{}]", jsonFileName);
}

void CameraEditor::SetParentTransform(const std::string& keyName, const Transform3D& parent) {

	// 無ければ処理できない
	auto it = keyObjects_.find(keyName);
	if (it != keyObjects_.end()) {

		it->second->SetParent(keyName, parent);
	}
}

void CameraEditor::StartAnim(const std::string& keyName, bool isAddFirstKey, bool isUpdateKey) {

	// 無ければ処理できない
	auto it = keyObjects_.find(keyName);
	if (it == keyObjects_.end()) {
		return;
	}

	// 再生中のカメラアニメーションがあれば終了させる
	for (auto& keyObject : std::views::values(keyObjects_)) {
		if (keyObject->IsUpdating()) {

			keyObject->Reset();
		}
	}

	// アクティブなキーオブジェクトに設定
	activeKeyObject_ = it->second.get();

	// 最初のキーを追加するかどうか
	// 追加する場合
	if (isAddFirstKey) {

		// シーンから現在のカメラ情報を取得
		BaseCamera* camera = sceneView_->GetCamera();

		// トランスフォームとfovY
		const Transform3D& cameraTransform = camera->GetTransform();
		float fovY = camera->GetFovY();
		std::vector<KeyframeObject3D::AnyValue> anyValues;
		anyValues.emplace_back(fovY);

		// キー情報の更新
		if (isUpdateKey) {

			activeKeyObject_->UpdateKey(true);
		}

		// 補間開始
		activeKeyObject_->StartLerp(cameraTransform, anyValues);
	}
	// 追加しない場合
	else {

		// キー情報の更新
		if (isUpdateKey) {

			activeKeyObject_->UpdateKey(true);
		}

		// 補間開始
		activeKeyObject_->StartLerp();
	}
}

void CameraEditor::EndAnim() {

	// 無ければ処理できない
	if (!activeKeyObject_) {
		return;
	}
	// リセットして非アクティブ状態にする
	activeKeyObject_->Reset();
	activeKeyObject_ = nullptr;

	// 更新を戻す
	BaseCamera* camera = sceneView_->GetCamera();
	camera->SetIsUpdateEditor(false);
}

bool CameraEditor::IsAnimFinished() const {

	// 無ければ終了しているとみなす
	if (!activeKeyObject_) {
		return true;
	}
	// 更新中でなければ終了しているのでtrueを返す
	return !activeKeyObject_->IsUpdating();
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

		//常に行う
		keyObject->UpdateKey();
	}

	// アクティブなキーオブジェクトの更新
	if (!activeKeyObject_) {
		return;
	}

	// 現在のゲームカメラ
	// カメラをエディターで更新中にする
	BaseCamera* camera = sceneView_->GetCamera();
	camera->SetIsUpdateEditor(true);

	// 時間を進めてキー更新
	activeKeyObject_->SelfUpdate();
	// カメラに適応
	ApplyToCamera(*sceneView_->GetCamera(), *activeKeyObject_);

	// 補間が終了したらアクティブ状態を解除
	if (!activeKeyObject_->IsUpdating()) {

		// 終了処理
		EndAnim();
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

		// 時間を渡して更新
		keyObjects_[selectedKeyObjectName_]->ExternalInputTUpdate(previewTimer_);

		// カメラへ適応
		ApplyToCamera(*camera, *keyObjects_[selectedKeyObjectName_].get());
		break;
	}
	case CameraEditor::PreviewMode::Play: {

		// 状態に応じた更新処理
		keyObjects_[selectedKeyObjectName_]->SelfUpdate();

		// カメラへ適応
		ApplyToCamera(*camera, *keyObjects_[selectedKeyObjectName_].get());

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

void CameraEditor::ApplyToCamera(BaseCamera& camera, const KeyframeObject3D& keyObject) {

	// 現在のキー位置のカメラ情報
	Transform3D transform = keyObject.GetCurrentTransform();

	float fovY = 0.0f;
	KeyframeObject3D::AnyValue fovValue = keyObject.GetCurrentAnyValue(addKeyValueFov_);
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

std::string CameraEditor::CheckName(const std::string& name) {

	int trailingNumber = 0;
	// ベースネームと末尾の数字に分離
	std::string base = SplitBaseNameAndNumber(name, trailingNumber);

	int& count = nameCounts_[base];

	if (trailingNumber > count) {
		count = trailingNumber;
	}

	std::string uniqueName;
	if (count == 0) {

		uniqueName = base;
	} else {

		uniqueName = base + std::to_string(count);
	}
	// 次回用に名前カウントを増やす
	count++;

	return uniqueName;
}

std::string CameraEditor::SplitBaseNameAndNumber(const std::string& name, int& number) {

	int idx = static_cast<int>(name.size()) - 1;
	while (idx >= 0 && std::isdigit(name[idx])) {
		idx--;
	}

	int startOfDigits = idx + 1;
	if (startOfDigits < static_cast<int>(name.size())) {

		// 末尾に数字がある場合
		number = std::stoi(name.substr(startOfDigits));
	} else {

		// 末尾に数字が無い場合
		number = 0;
	}

	return name.substr(0, startOfDigits);
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

		ImGui::SameLine();

		// 読み込み処理
		if (ImGui::Button("Load CameraKey")) {
			std::string outRelPath;
			if (ImGuiHelper::OpenJsonDialog(outRelPath)) {

				LoadJson(outRelPath, true);
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
	//	保存
	//================================================================================================================

	// 保存ボタン
	if (ImGui::Button("Save CameraKey")) {

		jsonSaveState_.showPopup = true;
	}
	// 保存処理
	{
		std::string outRelPath;
		if (ImGuiHelper::SaveJsonModal("Save CameraKey", jsonBasePath_.c_str(),
			jsonBasePath_.c_str(), jsonSaveState_, outRelPath)) {

			SaveJson(outRelPath);
		}
	}
	ImGui::Separator();

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
				if (previewKeyIndex_ < 0) {

					ImGui::TextDisabled("No Key Selected");
				} else {

					ImGui::Text("currentKeyIndex: %d", previewKeyIndex_);
				}
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

void CameraEditor::SaveJson(const std::string& fileName) {

	LOG_SCOPE_MS_LABEL("CameraEditorSaveJson");

	Json data;

	// キーオブジェクト
	keyObjects_[selectedKeyObjectName_]->ToJson(data["CameraKeyObject"]);
	// 名前
	data["keyName"] = selectedKeyObjectName_;

	// エディター
	data["previewMode_"] = EnumAdapter<PreviewMode>::ToString(previewMode_);
	data["previewLoopSpacing_"] = previewLoopSpacing_;

	JsonAdapter::Save(fileName, data);

	LOG_INFO("saved CameraKeyData: fileName: [{}]", fileName);
}