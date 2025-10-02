#include "Camera3DEditorPanel.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/System/Systems/TagSystem.h>
#include <Engine/Editor/ActionProgress/ActionProgressMonitor.h>

//============================================================================
//	Camera3DEditorPanel classMethods
//============================================================================

void Camera3DEditorPanel::Edit(std::unordered_map<std::string, CameraPathData>& params,
	std::unordered_map<std::string, CameraPathController::ActionSynchBind>& actionBinds,
	std::string& selectedObjectKey, std::string& selectedActionName,
	std::string& selectedParamKey, int& selectedKeyIndex,
	JsonSaveState& paramSaveState, char lastLoaded[128],
	CameraPathController::PlaybackState& playbackCamera) {

	float avail = ImGui::GetContentRegionAvail().x;
	float leftChild = avail * 0.5f - ImGui::GetStyle().ItemSpacing.x * 0.5f;
	float rightChild = avail * 0.5f;

	ImGui::SetWindowFontScale(0.8f);

	// アクションの選択、追加
	if (ImGuiHelper::BeginFramedChild("##SelectAdd", nullptr, ImVec2(leftChild, 128.0f))) {

		// 作成対象のアクション種類を選択
		SelectActionSubject(actionBinds, selectedObjectKey, selectedActionName);
		ImGui::Spacing();
		// カメラ調整項目の追加
		AddCameraParam(params, selectedActionName);
	}
	ImGuiHelper::EndFramedChild();
	ImGui::SameLine();

	// 値操作するカメラデータを選択
	if (ImGuiHelper::BeginFramedChild("##SelectParam", nullptr, ImVec2(rightChild, 128.0f))) {

		// どの調整項目の値を操作するか
		SelectCameraParam(params, selectedParamKey);
	}
	ImGuiHelper::EndFramedChild();
	ImGui::SetWindowFontScale(1.0f);

	if (ImGuiHelper::BeginFramedChild("##EditParam", nullptr,
		ImVec2(leftChild + rightChild, ImGui::GetContentRegionAvail().y))) {

		// 調整項目の値操作
		if (!params.empty() && !selectedParamKey.empty()) {

			EditCameraParam(params.at(selectedParamKey), actionBinds, selectedObjectKey, selectedParamKey,
				selectedKeyIndex, paramSaveState, lastLoaded, playbackCamera);
		}
	}
	ImGuiHelper::EndFramedChild();
}

void Camera3DEditorPanel::SelectActionSubject(
	std::unordered_map<std::string, CameraPathController::ActionSynchBind>& actionBinds,
	std::string& selectedObjectKey, std::string& selectedActionName) {

	// オブジェクトの選択
	std::vector<std::string> objectNames;
	objectNames.reserve(actionBinds.size());
	for (const auto& bind : actionBinds) {

		objectNames.emplace_back(bind.first);
	}
	int currentObjectIndex = -1;
	for (int i = 0; i < (int)objectNames.size(); ++i) {
		if (objectNames[i] == selectedObjectKey) {

			currentObjectIndex = i;
			break;
		}
	}
	// オブジェクトリストをコンボ表示
	if (ImGuiHelper::ComboFromStrings("Object", &currentObjectIndex, objectNames)) {
		if (0 <= currentObjectIndex && currentObjectIndex < static_cast<int>(objectNames.size())) {

			selectedObjectKey = objectNames[currentObjectIndex];
			selectedActionName.clear();
		}
	}

	// 進捗リストをIDから取得
	std::vector<std::string> overallNames;
	if (!selectedObjectKey.empty()) {

		ActionProgressMonitor* monitor = ActionProgressMonitor::GetInstance();
		overallNames = monitor->GetOverallNames(monitor->FindObjectID(selectedObjectKey));
	}

	int currentOverall = -1;
	const int overallCount = static_cast<int>(overallNames.size());
	for (int i = 0; i < overallCount; ++i) {
		if (overallNames[i] == selectedActionName) {
			currentOverall = i;
			break;
		}
	}
	// 進捗リストの中から選択
	if (ImGuiHelper::ComboFromStrings("Overall", &currentOverall, overallNames)) {
		if (0 <= currentOverall && currentOverall < overallCount) {

			selectedActionName = overallNames[currentOverall];
			auto it = actionBinds.find(selectedObjectKey);
			if (it != actionBinds.end()) {

				it->second.spanName = selectedActionName;
			}
		}
	}
}

void Camera3DEditorPanel::AddCameraParam(std::unordered_map<std::string, CameraPathData>& params,
	std::string& selectedAnimName) {

	// 何も設定されていなければ追加できない
	if (selectedAnimName.empty()) {
		return;
	}

	if (ImGui::Button("Add EditData")) {

		// 同じアニメーションは追加できないようにする
		if (Algorithm::Find(params, selectedAnimName)) {
			return;
		}

		CameraPathData param{};
		CameraPathData::KeyframeParam keyframe{};
		keyframe.Init();
		// キーフレームをデフォルトで1個追加
		param.keyframes.emplace_back(std::move(keyframe));

		// 調整項目追加
		params.emplace(selectedAnimName, std::move(param));
	}
}

void Camera3DEditorPanel::SelectCameraParam(std::unordered_map<std::string, CameraPathData>& params,
	std::string& selectedParamKey) {

	// params一覧
	std::vector<std::string> keys;
	keys.reserve(params.size());
	for (const auto& param : params) {

		keys.push_back(param.first);
	}

	// 現在のindexをkeysから逆引き
	int currentIndex = -1;
	if (!selectedParamKey.empty()) {
		for (int i = 0; i < static_cast<int>(keys.size()); ++i) {
			if (keys[i] == selectedParamKey) {

				currentIndex = i;
				break;
			}
		}
	}

	// 何もない場合は文字を表示する
	if (keys.empty()) {
		ImGui::TextDisabled("CameraParam is Empty");
		return;
	}

	// リスト選択
	if (ImGuiHelper::SelectableListFromStrings("CameraParam List", &currentIndex, keys, 8)) {
		if (currentIndex >= 0 && currentIndex < static_cast<int>(keys.size())) {

			selectedParamKey = keys[currentIndex];
		}
	}
}

void Camera3DEditorPanel::EditCameraParam(CameraPathData& param,
	std::unordered_map<std::string, CameraPathController::ActionSynchBind>& actionBinds,
	std::string& selectedObjectKey, std::string& selectedParamKey, int& selectedKeyIndex,
	JsonSaveState& paramSaveState, char lastLoaded[128],
	CameraPathController::PlaybackState& playbackCamera) {

	if (selectedParamKey.empty()) {
		ImGui::TextDisabled("not selected param");
		return;
	}

	// 保存と読み込み処理
	SaveAndLoad(param, paramSaveState, lastLoaded);

	// 選択キーの範囲補正
	if (selectedKeyIndex < 0 || static_cast<int>(param.keyframes.size()) <= selectedKeyIndex) {
		selectedKeyIndex = 0;
	}

	if (ImGui::BeginTabBar("EditCameraParam")) {
		if (ImGui::BeginTabItem("Playback")) {

			EditPlayback(param, playbackCamera, actionBinds, selectedObjectKey);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Lerp")) {

			EditLerp(param);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Keyframe")) {

			EditKeyframe(param, selectedKeyIndex);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Target")) {

			SelectTarget(param);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void Camera3DEditorPanel::SaveAndLoad(CameraPathData& param, JsonSaveState& paramSaveState, char lastLoaded[128]) {

	if (ImGui::Button("Save Param")) {

		paramSaveState.showPopup = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Load Param")) {

		std::string relPath;
		if (ImGuiHelper::OpenJsonDialog(relPath)) {

			param.ApplyJson(relPath);
			strncpy_s(lastLoaded, sizeof(lastLoaded), relPath.c_str(), _TRUNCATE);
		}
	}
	if (lastLoaded[0] != '\0') {

		ImGui::SameLine();
		ImGui::TextDisabled("Loaded: %s", lastLoaded);
	}

	// 保存処理
	{
		std::string outRelPath;
		if (ImGuiHelper::SaveJsonModal("Save CameraParam", param.cameraParamJsonPath.c_str(),
			param.cameraParamJsonPath.c_str(), paramSaveState, outRelPath)) {

			param.SaveJson(outRelPath);
		}
	}

	ImGui::Separator();
}

void Camera3DEditorPanel::EditPlayback(CameraPathData& param, CameraPathController::PlaybackState& playbackCamera,
	std::unordered_map<std::string, CameraPathController::ActionSynchBind>& actionBinds,
	std::string& selectedObjectKey) {

	ImGui::PushItemWidth(192.0f);

	ImGui::Checkbox("isActive", &playbackCamera.isActive);

	ImGui::SeparatorText("Synch");

	// 同期方向の設定
	if (!selectedObjectKey.empty()) {

		ImGui::Checkbox("driveStateFromCamera", &actionBinds[selectedObjectKey].driveStateFromCamera);
	}

	if (!playbackCamera.isActive) {
		return;
	}
	ImGui::Separator();
	ImGui::Spacing();

	EnumAdapter<CameraPathController::PreviewMode>::Combo("previewMode", &playbackCamera.mode);
	ImGui::Checkbox("isLoop", &playbackCamera.isLoop);
	ImGui::DragFloat("time", &playbackCamera.time, 0.001f, 0.0f, 1.0f);

	ImGui::SeparatorText("Param");

	ImGui::DragFloat("fovY", &param.keyframes[playbackCamera.selectedKeyIndex].fovY, 0.01f);

	if (playbackCamera.mode == CameraPathController::PreviewMode::Manual) {

		ImGui::SeparatorText("Keyframe");

		// 現在の位置にキーフレームを追加する
		if (ImGui::Button("Add CurrentCamera Keyframe")) {

			// 現在の区間を取得してその位置にキーフレームを追加する
			auto& keyframes = param.keyframes;
			const int keyCount = static_cast<int>(keyframes.size());
			// 現在の区間を取得
			float rawT = std::clamp(playbackCamera.time, 0.0f, 1.0f);
			float easedT = EasedValue(param.timer.easeingType_, rawT);
			float t = 0.0f;
			if (param.useAveraging && !param.averagedT.empty()) {

				t = LerpKeyframe::GetReparameterizedT(easedT, param.averagedT);
			} else {

				t = easedT;
			}
			// 区間インデックスを算出
			const float division = 1.0f / (std::max)(1, keyCount - 1);
			int segment = static_cast<int>(std::floor(t / division));
			segment = std::clamp(segment, 0, (std::max)(0, keyCount - 2));

			// 区間の値を取得
			Vector3 translation;
			Quaternion rotation;
			float fovY;
			{
				// 適応後のカメラ情報を取得する
				CameraPathController dummy(nullptr);
				dummy.Evaluate(param, t, translation, rotation, fovY);
			}
			// オフセットがあれば設定
			Vector3 offsetTranslation{};
			if (param.followTarget) {

				offsetTranslation = param.target->translation;
			}
			translation = translation - offsetTranslation;
			// キーフレームを初期化
			CameraPathData::KeyframeParam keyframe{};
			keyframe.Init();
			keyframe.demoObject->SetTranslation(translation);
			keyframe.demoObject->SetRotation(rotation);
			keyframe.translation = translation;
			keyframe.rotation = rotation;
			keyframe.fovY = fovY;
			// 追加
			keyframes.insert(keyframes.begin() + (segment + 1), std::move(keyframe));

			// キーフレームの平均値Tを再計算
			if (param.useAveraging) {

				param.averagedT = LerpKeyframe::AveragingPoints<Vector3>(
					param.CollectTranslationPoints(), param.divisionCount, param.lerpType);
			}
		}
	}

	ImGui::PopItemWidth();
}

void Camera3DEditorPanel::EditLerp(CameraPathData& param) {

	ImGui::PushItemWidth(192.0f);

	EnumAdapter<LerpKeyframe::Type>::Combo("LerpType", &param.lerpType);

	bool preUseAveraging = param.useAveraging;
	ImGui::Checkbox("averagingPoints", &param.useAveraging);
	ImGui::DragInt("divisionCount", &param.divisionCount, 1, 4, 512);
	if (preUseAveraging != param.useAveraging) {
		if (param.useAveraging) {

			param.averagedT = LerpKeyframe::AveragingPoints<Vector3>(
				param.CollectTranslationPoints(), param.divisionCount, param.lerpType);
		} else {

			param.averagedT.clear();
		}
	}

	ImGui::SeparatorText("Timer");

	param.timer.ImGui("Timer", false);

	ImGui::PopItemWidth();
}

void Camera3DEditorPanel::EditKeyframe(CameraPathData& param, int& selectedKeyIndex) {

	auto& keyframes = param.keyframes;
	int keyframeSize = static_cast<int>(keyframes.size());
	if (keyframes.empty()) {
		return;
	}
	// 選択中のキーフレーム
	ImGui::Text("selected Keyframe: %d / %d", selectedKeyIndex + 1, keyframeSize);

	// キーフレームの追加
	if (ImGui::Button("Add Keyframe")) {

		// 選択中のキーフレームの情報をコピー
		const CameraPathData::KeyframeParam& sourceKeyframe = keyframes[selectedKeyIndex];
		CameraPathData::KeyframeParam dstKeyframe{};
		dstKeyframe.fovY = sourceKeyframe.fovY;

		// デモ用オブジェクトを作成
		dstKeyframe.demoObject = std::make_unique<GameObject3D>();
		dstKeyframe.demoObject->Init("demoCamera", "demoCamera", "Editor");

		Json data;
		if (!JsonAdapter::LoadCheck(param.demoCameraJsonPath, data)) {
			return;
		}
		// 見た目を設定
		dstKeyframe.demoObject->ApplyTransform(data);
		dstKeyframe.demoObject->ApplyMaterial(data);
		dstKeyframe.demoObject->SetMeshRenderView(MeshRenderView::Scene);

		// 追加
		keyframes.insert(keyframes.begin() + (selectedKeyIndex + 1), std::move(dstKeyframe));
		selectedKeyIndex += 1;
	}
	ImGui::SameLine();

	// キーフレームの削除
	if (ImGui::Button("Remove Keyframe")) {

		keyframes.erase(keyframes.begin() + selectedKeyIndex);
		if (keyframeSize <= selectedKeyIndex) {
			selectedKeyIndex = keyframeSize - 1;
		}
	}
	// キーフレームの順番入れ替え
	ImGui::Separator();
	ImGui::TextUnformatted("Reorder");
	bool canUp = (selectedKeyIndex > 0);
	bool canDown = (selectedKeyIndex + 1 < keyframeSize);

	if (!canUp) {
		ImGui::BeginDisabled();
	}
	if (ImGui::Button("Move Up")) {

		std::swap(keyframes[selectedKeyIndex - 1], keyframes[selectedKeyIndex]);
		selectedKeyIndex -= 1;
	}
	if (!canUp) {
		ImGui::EndDisabled();
	}

	ImGui::SameLine();

	if (!canDown) {
		ImGui::BeginDisabled();
	}
	if (ImGui::Button("Move Down")) {

		std::swap(keyframes[selectedKeyIndex], keyframes[selectedKeyIndex + 1]);
		selectedKeyIndex += 1;
	}
	if (!canDown) {
		ImGui::EndDisabled();
	}
}

void Camera3DEditorPanel::SelectTarget(CameraPathData& param) {

	ImGui::Text("currentTarget: %s", param.targetName.c_str());
	ImGui::Checkbox("followTarget", &param.followTarget);
	ImGui::Checkbox("followRotation", &param.followRotation);
	if (ImGui::Button("Remove Target")) {

		param.target = nullptr;
		param.targetName = "";
	}
	ImGui::Separator();

	uint32_t currentId = 0;
	ObjectManager* objectManager = ObjectManager::GetInstance();
	TagSystem* tagSystem = objectManager->GetSystem<TagSystem>();
	// 現在選択されているオブジェクトIDを設定
	for (const auto& [id, tagPtr] : tagSystem->Tags()) {
		if (objectManager->GetData<Transform3D>(id) == param.target) {

			currentId = id;
			break;
		}
	}

	std::string selectedName = param.targetName;
	if (ImGuiHelper::SelectTagTarget("Select Follow Target", &currentId, &selectedName)) {

		// Transformと名前を更新
		param.target = objectManager->GetData<Transform3D>(currentId);
		param.targetName = selectedName;
	}
}